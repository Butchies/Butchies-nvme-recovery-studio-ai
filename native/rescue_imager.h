#pragma once
// Automatic rescue imaging for externally connected USB SSDs.
//
// The source device is opened strictly read-only (GENERIC_READ). Data is
// written only into a new folder on a separate destination volume, which is
// verified not to reside on the source disk. Progress is persisted in a
// ddrescue-compatible map file so that an interrupted rescue can be resumed.
//
// Passes:
//   1  Copy:     large blocks forward, unreadable blocks are skipped with
//                growing skip distance (healthy areas first).
//   2  Fill:     skipped areas are read without skipping.
//   3  Trim:     failed blocks are read sector by sector from both edges.
//   4  Scrape:   remaining inner parts of failed blocks, sector by sector.
//   5+ Retry:    bad sectors are retried as configured by the plan.
//   Then:        map/coverage check, SHA-256 of the image, report.
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QVariantList>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QStorageInfo>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QSysInfo>
#include <QCryptographicHash>
#include <QElapsedTimer>
#include <QRegularExpression>
#include <QThread>
#include <atomic>
#include <functional>
#include <optional>
#include <vector>
#include <algorithm>
#include "platform_io.h"

namespace RescueImager {

constexpr char NonTried='?';
constexpr char NonTrimmed='*';
constexpr char NonScraped='/';
constexpr char BadSector='-';
constexpr char Finished='+';

struct Region { quint64 pos=0; quint64 size=0; char status=NonTried; };

// Sorted, contiguous list of regions covering [0,total).
class RescueMap {
public:
    void reset(quint64 total) { m_total=total; m_regions.clear(); if(total) m_regions.push_back({0,total,NonTried}); }
    quint64 total() const { return m_total; }
    const std::vector<Region> &regions() const { return m_regions; }

    void set(quint64 pos, quint64 size, char status) {
        if(size==0 || pos>=m_total) return;
        size=qMin(size,m_total-pos);
        const quint64 end=pos+size;
        std::vector<Region> out;
        out.reserve(m_regions.size()+2);
        bool inserted=false;
        for(const auto &r:m_regions) {
            const quint64 rEnd=r.pos+r.size;
            if(r.pos<pos) out.push_back({r.pos,qMin(rEnd,pos)-r.pos,r.status});
            if(!inserted && rEnd>pos) { out.push_back({pos,size,status}); inserted=true; }
            if(rEnd>end) { const quint64 s=qMax(r.pos,end); out.push_back({s,rEnd-s,r.status}); }
        }
        m_regions.clear();
        for(const auto &r:out) {
            if(r.size==0) continue;
            if(!m_regions.empty() && m_regions.back().status==r.status)
                m_regions.back().size+=r.size;
            else m_regions.push_back(r);
        }
    }
    std::optional<Region> next(char status, quint64 from) const {
        for(const auto &r:m_regions)
            if(r.status==status && r.pos+r.size>from) return r;
        return std::nullopt;
    }
    std::vector<Region> all(char status) const {
        std::vector<Region> v;
        for(const auto &r:m_regions) if(r.status==status) v.push_back(r);
        return v;
    }
    quint64 bytes(char status) const {
        quint64 n=0; for(const auto &r:m_regions) if(r.status==status) n+=r.size; return n;
    }
    int count(char status) const {
        int n=0; for(const auto &r:m_regions) if(r.status==status) ++n; return n;
    }
    QString serialize(quint64 currentPos, char currentStatus, int pass) const {
        QString text;
        QTextStream s(&text);
        s << "# Mapfile. Created by NVMe Recovery Studio AI (ddrescue-compatible)\n";
        s << "# " << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n";
        s << "# current_pos  current_status  current_pass\n";
        s << hex(currentPos) << "     " << currentStatus << "               " << pass << "\n";
        s << "#      pos        size  status\n";
        for(const auto &r:m_regions) s << hex(r.pos) << "  " << hex(r.size) << "  " << r.status << "\n";
        return text;
    }
    bool parse(const QString &text, quint64 expectedTotal) {
        std::vector<Region> regions;
        bool statusLineSeen=false;
        const auto lines=text.split(QLatin1Char('\n'));
        for(auto line:lines) {
            line=line.trimmed();
            if(line.isEmpty() || line.startsWith(QLatin1Char('#'))) continue;
            const auto parts=line.split(QRegularExpression(QStringLiteral("\\s+")),Qt::SkipEmptyParts);
            if(!statusLineSeen) { statusLineSeen=true; continue; }
            if(parts.size()<3 || parts[2].size()!=1) return false;
            bool ok1=false, ok2=false;
            const quint64 pos=parts[0].toULongLong(&ok1,0);
            const quint64 size=parts[1].toULongLong(&ok2,0);
            const char st=parts[2].at(0).toLatin1();
            if(!ok1 || !ok2 || size==0) return false;
            if(QStringLiteral("?*/-+").indexOf(QLatin1Char(st))<0) return false;
            const quint64 expectedPos=regions.empty() ? 0 : regions.back().pos+regions.back().size;
            if(pos!=expectedPos) return false;
            regions.push_back({pos,size,st});
        }
        if(regions.empty() || regions.back().pos+regions.back().size!=expectedTotal) return false;
        m_total=expectedTotal;
        m_regions=regions;
        return true;
    }
private:
    static QString hex(quint64 v) { return QStringLiteral("0x%1").arg(qulonglong(v),8,16,QLatin1Char('0')); }
    quint64 m_total=0;
    std::vector<Region> m_regions;
};

struct Config {
    QString sourcePath;            // \\.\PHYSICALDRIVEn
    QString destinationDir;        // folder on a separate volume
    QVariantMap device;            // verified inventory data
    QVariantMap diagnosis;         // RecoveryAgent::analyze result
    QVariantMap plan;              // RecoveryAgent::planRescue result
};

using ProgressFn=std::function<void(const QVariantMap&)>;

inline QString sanitize(QString text) {
    text=text.trimmed();
    text.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9_-]+")),QStringLiteral("_"));
    text=text.left(40);
    return text.isEmpty() ? QStringLiteral("unbekannt") : text;
}

inline QString formatBytes(quint64 v) {
    const char *units[]={"B","KiB","MiB","GiB","TiB"};
    double d=double(v); int u=0;
    while(d>=1024.0 && u<4) { d/=1024.0; ++u; }
    return QStringLiteral("%1 %2").arg(d,0,'f',u==0?0:2).arg(QLatin1String(units[u]));
}

inline QVariantMap run(const Config &cfg, std::atomic_bool &cancel, const ProgressFn &progress) {
    QVariantMap result;
    result["state"]="fehler";
    result["ok"]=false;
    const auto fail=[&](const QString &message) { result["message"]=message; return result; };
    const auto &plan=cfg.plan;
    if(!plan.value("allowed").toBool())
        return fail(plan.value("reason",QStringLiteral("Rettung durch Sicherheitsregeln gesperrt.")).toString());
    if(cfg.sourcePath.trimmed().isEmpty()) return fail(QStringLiteral("Kein Quelllaufwerk angegeben."));

    // --- Destination checks -------------------------------------------------
    const QFileInfo destInfo(cfg.destinationDir);
    if(cfg.destinationDir.trimmed().isEmpty() || !destInfo.exists() || !destInfo.isDir() || destInfo.isSymLink())
        return fail(QStringLiteral("Bitte einen gültigen Zielordner auf einem anderen Laufwerk angeben."));
    QString error;
    if(!PlatformIO::destinationOnSeparateDisk(cfg.destinationDir,cfg.sourcePath,error)) return fail(error);

    // --- Source -------------------------------------------------------------
    PlatformIO::SourceDevice src;
    if(!src.open(cfg.sourcePath,error)) return fail(error);
    const quint64 total=src.size();
    const quint32 sector=src.sectorSize();
    if(total==0) return fail(QStringLiteral("SSD meldet Kapazität 0."));

    const QStorageInfo volume(cfg.destinationDir);
    if(!volume.isValid() || !volume.isReady() || volume.isReadOnly())
        return fail(QStringLiteral("Ziellaufwerk nicht beschreibbar."));
    const QString fs=QString::fromLatin1(volume.fileSystemType()).toUpper();
    if(fs.contains(QStringLiteral("FAT")) && !fs.contains(QStringLiteral("EXFAT")) && total>=4ULL*1024*1024*1024)
        return fail(QStringLiteral("Ziellaufwerk ist FAT32 (max. 4 GiB je Datei). Bitte NTFS- oder exFAT-Laufwerk verwenden."));

    // --- Case folder, provenance, resume -----------------------------------
    const QString serial=cfg.device.value("nvmeControllerSerial").toString()!=QStringLiteral("Nicht ermittelt")
        ? cfg.device.value("nvmeControllerSerial").toString() : cfg.device.value("serial").toString();
    const QString model=cfg.device.value("nvmeControllerModel").toString()!=QStringLiteral("Nicht ermittelt")
        ? cfg.device.value("nvmeControllerModel").toString() : cfg.device.value("name").toString();
    QDir folder(cfg.destinationDir);
    const QString folderName=QStringLiteral("NVMe-Rettung_%1_%2").arg(sanitize(model),sanitize(serial));
    if(!folder.exists(folderName) && !folder.mkdir(folderName))
        return fail(QStringLiteral("Rettungsordner kann nicht angelegt werden."));
    folder.cd(folderName);
    const QString imagePath=folder.filePath(QStringLiteral("abbild.img"));
    const QString mapPath=folder.filePath(QStringLiteral("abbild.map"));
    const QString originPath=folder.filePath(QStringLiteral("herkunft.json"));
    const QString logPath=folder.filePath(QStringLiteral("protokoll.log"));
    result["folder"]=folder.absolutePath();
    result["imagePath"]=imagePath;
    result["mapPath"]=mapPath;

    QFile logFile(logPath);
    logFile.open(QIODevice::Append|QIODevice::Text);
    const auto event=[&](const QString &text) {
        const QString line=QStringLiteral("%1  %2").arg(QDateTime::currentDateTime().toString(Qt::ISODate),text);
        if(logFile.isOpen()) { logFile.write(line.toUtf8()+"\n"); logFile.flush(); }
        progress(QVariantMap{{"event",line}});
    };

    QJsonObject origin;
    origin["programm"]="NVMe Recovery Studio AI";
    origin["anschluss"]="USB";
    origin["quellpfad"]=cfg.sourcePath;
    origin["modell"]=model;
    origin["seriennummer"]=serial;
    origin["usbSeriennummer"]=cfg.device.value("serial").toString();
    origin["nvmeFirmware"]=cfg.device.value("nvmeFirmware").toString();
    origin["pnpDeviceId"]=cfg.device.value("pnpDeviceId").toString();
    origin["kapazitaetBytes"]=QString::number(total);
    origin["sektorgroesse"]=int(sector);
    origin["rechner"]=QSysInfo::machineHostName();

    RescueMap map;
    bool resume=false;
    if(QFileInfo::exists(originPath)) {
        QFile f(originPath);
        if(!f.open(QIODevice::ReadOnly)) return fail(QStringLiteral("Vorhandene Herkunftsdatei nicht lesbar."));
        const auto old=QJsonDocument::fromJson(f.readAll()).object();
        const bool same=old.value("kapazitaetBytes").toString()==origin.value("kapazitaetBytes").toString()
            && old.value("sektorgroesse").toInt()==int(sector)
            && old.value("seriennummer").toString()==serial
            && old.value("modell").toString()==model;
        if(!same) return fail(QStringLiteral("Konflikt: Der Zielordner enthält ein Abbild einer anderen SSD. Kein Zusammenführen; bitte anderen Zielordner wählen."));
        QFile m(mapPath);
        if(!QFileInfo::exists(imagePath) || !m.open(QIODevice::ReadOnly|QIODevice::Text) || !map.parse(QString::fromUtf8(m.readAll()),total))
            return fail(QStringLiteral("Vorhandenes Abbild oder Lesekarte beschädigt. Bitte anderen Zielordner wählen."));
        origin["erstellt"]=old.value("erstellt");
        resume=true;
    } else {
        if(QFileInfo::exists(imagePath))
            return fail(QStringLiteral("Zielordner enthält ein Abbild unbekannter Herkunft; es wird nicht überschrieben."));
        if(quint64(volume.bytesAvailable())<total)
            return fail(QStringLiteral("Zu wenig Platz auf dem Ziel: benötigt %1, frei %2.")
                .arg(formatBytes(total),formatBytes(quint64(volume.bytesAvailable()))));
        map.reset(total);
        origin["erstellt"]=QDateTime::currentDateTime().toString(Qt::ISODate);
    }
    {
        QSaveFile f(originPath);
        if(!f.open(QIODevice::WriteOnly) || f.write(QJsonDocument(origin).toJson())<0 || !f.commit())
            return fail(QStringLiteral("Herkunftsdatei kann nicht geschrieben werden."));
    }

    // --- Image file ----------------------------------------------------------
    PlatformIO::ImageFile image;
    if(!image.open(imagePath,!resume,error)) return fail(error);
    if(!resume && !image.resize(total))
        return fail(QStringLiteral("Abbilddatei kann nicht auf SSD-Größe gebracht werden (%1).").arg(image.lastError()));

    // --- Plan parameters ----------------------------------------------------
    const auto alignUp=[&](quint64 v){ return qMax<quint64>(sector,(v+sector-1)/sector*sector); };
    const quint64 chunk=qMin<quint64>(alignUp(plan.value("chunkBytes",1048576).toULongLong()),16ULL*1024*1024);
    const quint64 skipStart=alignUp(plan.value("skipStartBytes",1048576).toULongLong());
    const quint64 skipMax=alignUp(plan.value("skipMaxBytes",268435456ULL).toULongLong());
    const quint32 timeoutMs=plan.value("readTimeoutMs",30000).toUInt();
    const int retryPasses=qBound(0,plan.value("retryPasses",1).toInt(),5);
    const int maxConsecutiveTimeouts=qBound(1,plan.value("maxConsecutiveTimeouts",3).toInt(),20);
    const unsigned long reconnectWait=qBound(1,plan.value("reconnectWaitSeconds",3).toInt(),60);

    void *buffer=PlatformIO::allocAligned(size_t(chunk),sector);
    if(!buffer) return fail(QStringLiteral("Lesepuffer kann nicht angelegt werden."));
    struct BufferGuard { void *p; PlatformIO::SourceDevice *s; ~BufferGuard(){ if(!s->hung()) PlatformIO::freeAligned(p); } } bufferGuard{buffer,&src};

    event(resume ? QStringLiteral("Rettung wird fortgesetzt: %1 bereits gesichert.").arg(formatBytes(map.bytes(Finished)))
                 : QStringLiteral("Neues Rettungsabbild: %1, Sektorgröße %2 Byte.").arg(formatBytes(total)).arg(sector));
    for(const auto &d:plan.value("decisions").toList()) event(QStringLiteral("Entscheidung: %1").arg(d.toString()));

    // --- Progress and persistence -------------------------------------------
    QElapsedTimer sessionTimer; sessionTimer.start();
    QElapsedTimer emitTimer; emitTimer.start();
    QElapsedTimer saveTimer; saveTimer.start();
    const quint64 finishedAtStart=map.bytes(Finished);
    int pass=0; QString phase; quint64 currentPos=0; int readErrors=0; int consecutiveTimeouts=0;
    int timeouts=0; int deviceLost=0;
    const auto saveMap=[&](char st)->bool {
        image.flush();
        QSaveFile f(mapPath);
        return f.open(QIODevice::WriteOnly|QIODevice::Text) && f.write(map.serialize(currentPos,st,pass).toUtf8())>=0 && f.commit();
    };
    const auto report=[&](bool force) {
        if(!force && emitTimer.elapsed()<500) return;
        emitTimer.restart();
        const quint64 done=map.bytes(Finished);
        const quint64 open=map.bytes(NonTried)+map.bytes(NonTrimmed)+map.bytes(NonScraped);
        const double secs=qMax(0.001,sessionTimer.elapsed()/1000.0);
        const double rate=double(done-finishedAtStart)/secs;
        QVariantMap p;
        p["phase"]=phase; p["pass"]=pass;
        p["percent"]=100.0*double(total-open)/double(total);
        p["rescuedPercent"]=100.0*double(done)/double(total);
        p["rescued"]=formatBytes(done);
        p["bad"]=formatBytes(map.bytes(BadSector));
        p["badRegions"]=map.count(BadSector);
        p["pending"]=formatBytes(open);
        p["rate"]=QStringLiteral("%1/s").arg(formatBytes(quint64(rate)));
        p["eta"]=(rate>1.0 && pass<=2) ? QStringLiteral("%1 min").arg(qRound(double(map.bytes(NonTried))/rate/60.0)) : QStringLiteral("–");
        p["readErrors"]=readErrors;
        progress(p);
        if(saveTimer.elapsed()>=30000) { saveTimer.restart(); saveMap(NonTried); }
    };

    enum Outcome { Ok, Cancelled, Fatal };
    QString fatalMessage;
    // Reads one block. Returns Ok (data written), Cancelled, Fatal or sets readFailed.
    const auto readBlock=[&](quint64 pos,quint64 len,bool &readFailed)->Outcome {
        readFailed=false;
        if(cancel.load()) return Cancelled;
        currentPos=pos;
        using PlatformIO::ReadStatus;
        auto r=src.read(pos,buffer,quint32(len),timeoutMs);
        if(r.status==ReadStatus::Hung) {
            ++timeouts;
            fatalMessage=QStringLiteral("Die SSD reagiert nicht mehr auf Lesebefehle. Fortschritt ist gespeichert. SSD trennen bzw. Rechner neu starten, abkühlen lassen und Rettung fortsetzen.");
            return Fatal;
        }
        if(r.status==ReadStatus::DeviceLost) {
            event(QStringLiteral("Verbindung zur SSD verloren (%1); neuer Verbindungsversuch.").arg(PlatformIO::describe(r)));
            ++deviceLost;
            QThread::sleep(reconnectWait);
            if(!src.reopen() || src.size()!=total || src.sectorSize()!=sector) {
                fatalMessage=QStringLiteral("SSD nicht mehr erreichbar. Fortschritt ist gespeichert; nach dem Neuverbinden kann die Rettung fortgesetzt werden.");
                return Fatal;
            }
            r=src.read(pos,buffer,quint32(len),timeoutMs);
            if(r.status==ReadStatus::Hung || r.status==ReadStatus::DeviceLost) { fatalMessage=QStringLiteral("SSD nach Neuverbindung nicht stabil lesbar. Fortschritt ist gespeichert."); return Fatal; }
        }
        if(r.status==ReadStatus::Rejected) { fatalMessage=QStringLiteral("Lesebefehl vom Gerät abgelehnt (Ausrichtung/Parameter)."); return Fatal; }
        if(r.status==ReadStatus::Ok) {
            consecutiveTimeouts=0;
            if(!image.write(pos,buffer,quint32(len))) { fatalMessage=QStringLiteral("Schreiben auf das Ziellaufwerk fehlgeschlagen (%1). Ziel voll oder getrennt?").arg(image.lastError()); return Fatal; }
            map.set(pos,len,Finished);
            return Ok;
        }
        readFailed=true; ++readErrors;
        if(r.status==ReadStatus::Timeout) {
            ++timeouts;
            if(++consecutiveTimeouts>=maxConsecutiveTimeouts) {
                fatalMessage=QStringLiteral("%1 Zeitüberschreitungen in Folge: Die SSD wird geschont und die Rettung pausiert. Fortschritt ist gespeichert; später fortsetzen.").arg(consecutiveTimeouts);
                return Fatal;
            }
        } else consecutiveTimeouts=0;
        if(readErrors<=50 || readErrors%1000==0)
            event(QStringLiteral("Lesefehler bei Byte %1 (%2): %3").arg(pos).arg(formatBytes(len)).arg(PlatformIO::describe(r)));
        return Ok;
    };

    const auto copyPass=[&](bool allowSkip)->Outcome {
        quint64 pos=0, skip=0;
        while(auto r=map.next(NonTried,pos)) {
            const quint64 start=qMax(r->pos,pos);
            const quint64 end=r->pos+r->size;
            const quint64 len=qMin(chunk,end-start);
            bool failed=false;
            const auto o=readBlock(start,len,failed);
            if(o!=Ok) return o;
            pos=start+len;
            if(failed) {
                map.set(start,len,NonTrimmed);
                if(allowSkip && pos<end) {
                    skip=skip ? qMin(skip*2,skipMax) : skipStart;
                    pos+=qMin(skip,end-pos);
                }
            } else skip=0;
            report(false);
        }
        return Ok;
    };
    const auto sectorPass=[&](char wanted,char failStatus)->Outcome {
        for(const auto &r:map.all(wanted)) {
            for(quint64 p=r.pos;p<r.pos+r.size;p+=sector) {
                bool failed=false;
                const auto o=readBlock(p,sector,failed);
                if(o!=Ok) return o;
                if(failed) map.set(p,sector,failStatus);
                report(false);
            }
        }
        return Ok;
    };
    const auto trimPass=[&]()->Outcome {
        for(const auto &r:map.all(NonTrimmed)) {
            quint64 a=r.pos, b=r.pos+r.size;
            bool failed=false;
            while(a<b) {
                const auto o=readBlock(a,sector,failed); if(o!=Ok) return o;
                if(failed) { map.set(a,sector,BadSector); a+=sector; break; }
                a+=sector; report(false);
            }
            while(b>a) {
                const auto o=readBlock(b-sector,sector,failed); if(o!=Ok) return o;
                if(failed) { map.set(b-sector,sector,BadSector); b-=sector; break; }
                b-=sector; report(false);
            }
            if(b>a) map.set(a,b-a,NonScraped);
        }
        return Ok;
    };

    struct Step { int pass; QString name; std::function<Outcome()> fn; };
    std::vector<Step> steps{
        {1,QStringLiteral("Durchgang 1: Schnellkopie gesunder Bereiche"),[&]{ return copyPass(true); }},
        {2,QStringLiteral("Durchgang 2: Übersprungene Bereiche lesen"),[&]{ return copyPass(false); }},
        {3,QStringLiteral("Durchgang 3: Fehlerbereiche eingrenzen"),[&]{ return trimPass(); }},
        {4,QStringLiteral("Durchgang 4: Einzelsektoren auslesen"),[&]{ return sectorPass(NonScraped,BadSector); }},
    };
    for(int i=0;i<retryPasses;++i)
        steps.push_back({5+i,QStringLiteral("Durchgang %1: Defekte Sektoren erneut versuchen").arg(5+i),[&]{ return sectorPass(BadSector,BadSector); }});

    Outcome outcome=Ok;
    for(const auto &step:steps) {
        pass=step.pass; phase=step.name;
        event(phase);
        report(true);
        outcome=step.fn();
        saveMap(outcome==Ok ? Finished : NonTried);
        if(outcome!=Ok) break;
    }
    report(true);

    // --- Result and verification -------------------------------------------
    const quint64 done=map.bytes(Finished);
    const quint64 bad=map.bytes(BadSector);
    const quint64 open=map.bytes(NonTried)+map.bytes(NonTrimmed)+map.bytes(NonScraped);
    result["totalBytes"]=QString::number(total);
    result["rescuedBytes"]=QString::number(done);
    result["badBytes"]=QString::number(bad);
    result["openBytes"]=QString::number(open);
    result["badRegions"]=map.count(BadSector);
    result["rescuedPercent"]=100.0*double(done)/double(total);
    result["readErrors"]=readErrors;
    result["timeouts"]=timeouts;
    result["deviceLost"]=deviceLost;
    result["rateBytesPerSec"]=double(done-finishedAtStart)/qMax(0.001,sessionTimer.elapsed()/1000.0);
    result["sectorSize"]=int(sector);
    QVariantList badList;
    for(const auto &r:map.all(BadSector)) {
        if(badList.size()>=200) break;
        badList.append(QStringLiteral("Byte %1 – %2 (%3)").arg(r.pos).arg(r.pos+r.size-1).arg(formatBytes(r.size)));
    }
    result["badList"]=badList;

    const bool sizeOk=image.size()==total;
    result["imageSizeOk"]=sizeOk;
    if(!sizeOk) event(QStringLiteral("Prüfung: Abbildgröße stimmt nicht mit der SSD-Kapazität überein."));

    if(outcome==Cancelled) { result["state"]="abgebrochen"; result["message"]=QStringLiteral("Rettung auf Wunsch angehalten. Fortsetzen jederzeit mit demselben Zielordner."); }
    else if(outcome==Fatal) { result["state"]="unterbrochen"; result["message"]=fatalMessage; }
    else {
        result["state"]="abgeschlossen";
        result["message"]=bad==0 ? QStringLiteral("Alle Sektoren wurden gelesen.") : QStringLiteral("Rettung abgeschlossen; einzelne Sektoren sind nicht lesbar.");
        // Integrity hash of the finished image (destination only; the SSD is not read again).
        image.flush();
        phase=QStringLiteral("Prüfsumme des Abbilds (SHA-256)");
        event(phase);
        QFile img(imagePath);
        if(img.open(QIODevice::ReadOnly)) {
            QCryptographicHash hash(QCryptographicHash::Sha256);
            quint64 hashed=0; QElapsedTimer t; t.start();
            QByteArray block;
            bool aborted=false;
            while(!(block=img.read(8*1024*1024)).isEmpty()) {
                if(cancel.load()) { aborted=true; break; }
                hash.addData(block); hashed+=quint64(block.size());
                if(t.elapsed()>=500) { t.restart(); progress(QVariantMap{{"phase",phase},{"percent",100.0*double(hashed)/double(total)}}); }
            }
            if(!aborted && hashed==total) {
                result["sha256"]=QString::fromLatin1(hash.result().toHex());
                QFile h(folder.filePath(QStringLiteral("abbild.img.sha256")));
                if(h.open(QIODevice::WriteOnly|QIODevice::Text))
                    h.write(result["sha256"].toString().toUtf8()+" *abbild.img\n");
                event(QStringLiteral("SHA-256: %1").arg(result["sha256"].toString()));
            } else event(QStringLiteral("Prüfsumme nicht vollständig berechnet."));
        }
    }
    result["ok"]=outcome==Ok;
    event(QStringLiteral("Ergebnis: %1 von %2 gesichert (%3 %), %4 nicht lesbar in %5 Bereichen.")
        .arg(formatBytes(done),formatBytes(total)).arg(result["rescuedPercent"].toDouble(),0,'f',3)
        .arg(formatBytes(bad)).arg(map.count(BadSector)));
    return result;
}

// Writes bericht.txt and bericht.json into the rescue folder.
inline QString writeReport(const QString &folder, const QVariantMap &device, const QVariantMap &diagnosis,
                           const QVariantMap &plan, const QVariantMap &result, const QVariantMap &evaluation) {
    if(folder.isEmpty()) return {};
    QDir dir(folder);
    QJsonObject json;
    json["erstellt"]=QDateTime::currentDateTime().toString(Qt::ISODate);
    json["geraet"]=QJsonObject::fromVariantMap(device);
    json["diagnose"]=QJsonObject::fromVariantMap(diagnosis);
    json["plan"]=QJsonObject::fromVariantMap(plan);
    json["ergebnis"]=QJsonObject::fromVariantMap(result);
    json["bewertung"]=QJsonObject::fromVariantMap(evaluation);
    QSaveFile jf(dir.filePath(QStringLiteral("bericht.json")));
    if(jf.open(QIODevice::WriteOnly)) { jf.write(QJsonDocument(json).toJson()); jf.commit(); }

    QString t;
    QTextStream s(&t);
    s << QStringLiteral("NVMe Recovery Studio AI – Rettungsbericht\n");
    s << QStringLiteral("Erstellt: ") << QDateTime::currentDateTime().toString(QStringLiteral("dd.MM.yyyy HH:mm:ss")) << QStringLiteral("\n\n");
    s << QStringLiteral("GERÄT\n");
    s << QStringLiteral("  Modell:        ") << device.value("nvmeControllerModel").toString() << QStringLiteral(" / ") << device.value("name").toString() << QStringLiteral("\n");
    s << QStringLiteral("  Seriennummer:  ") << device.value("nvmeControllerSerial").toString() << QStringLiteral("\n");
    s << QStringLiteral("  Firmware:      ") << device.value("nvmeFirmware").toString() << QStringLiteral("\n");
    if(device.value("connection").toString()==QStringLiteral("PCIe"))
        s << QStringLiteral("  Anschluss:     PCIe ") << device.value("pciAddress").toString() << QStringLiteral(" (") << device.value("pciVendorId").toString() << QStringLiteral(":") << device.value("pciDeviceId").toString() << QStringLiteral(")\n\n");
    else
        s << QStringLiteral("  Anschluss:     USB, VID ") << device.value("bridgeVendorId").toString() << QStringLiteral(" PID ") << device.value("bridgeProductId").toString() << QStringLiteral("\n\n");
    s << QStringLiteral("DIAGNOSE\n  ") << diagnosis.value("diagnosis").toString() << QStringLiteral("\n");
    s << QStringLiteral("  Bewertung: ") << diagnosis.value("severity").toString() << QStringLiteral(" · Sicherheit: ") << diagnosis.value("confidence").toString() << QStringLiteral("\n");
    for(const auto &e:diagnosis.value("evidence").toList()) s << QStringLiteral("  Beleg: ") << e.toString() << QStringLiteral("\n");
    for(const auto &w:diagnosis.value("warnings").toList()) s << QStringLiteral("  Warnung: ") << w.toString() << QStringLiteral("\n");
    s << QStringLiteral("\nENTSCHEIDUNGEN DER RECOVERY-STEUERUNG\n");
    for(const auto &d:plan.value("decisions").toList()) s << QStringLiteral("  • ") << d.toString() << QStringLiteral("\n");
    s << QStringLiteral("\nERGEBNIS\n");
    s << QStringLiteral("  Status:        ") << result.value("state").toString() << QStringLiteral(" – ") << result.value("message").toString() << QStringLiteral("\n");
    s << QStringLiteral("  Gesichert:     ") << formatBytes(result.value("rescuedBytes").toULongLong()) << QStringLiteral(" von ")
      << formatBytes(result.value("totalBytes").toULongLong())
      << QStringLiteral(" (%1 %)").arg(result.value("rescuedPercent").toDouble(),0,'f',3) << QStringLiteral("\n");
    s << QStringLiteral("  Nicht lesbar:  ") << formatBytes(result.value("badBytes").toULongLong()) << QStringLiteral(" in ") << result.value("badRegions").toInt() << QStringLiteral(" Bereichen\n");
    s << QStringLiteral("  Offen:         ") << formatBytes(result.value("openBytes").toULongLong()) << QStringLiteral("\n");
    s << QStringLiteral("  Abbildgröße:   ") << (result.value("imageSizeOk").toBool() ? QStringLiteral("korrekt") : QStringLiteral("abweichend")) << QStringLiteral("\n");
    if(result.contains("sha256")) s << QStringLiteral("  SHA-256:       ") << result.value("sha256").toString() << QStringLiteral("\n");
    s << QStringLiteral("\nBEWERTUNG\n  ") << evaluation.value("summary").toString() << QStringLiteral("\n");
    for(const auto &n:evaluation.value("nextSteps").toList()) s << QStringLiteral("  → ") << n.toString() << QStringLiteral("\n");
    const auto badList=result.value("badList").toList();
    if(!badList.isEmpty()) {
        s << QStringLiteral("\nNICHT LESBARE BEREICHE") << (badList.size()>=200 ? QStringLiteral(" (erste 200)") : QString()) << QStringLiteral("\n");
        for(const auto &b:badList) s << QStringLiteral("  ") << b.toString() << QStringLiteral("\n");
    }
    s << QStringLiteral("\nDie Quell-SSD wurde ausschließlich gelesen. Es wurden keine Firmware-, Reset- oder Schreibbefehle an die SSD gesendet.\n");
    QSaveFile tf(dir.filePath(QStringLiteral("bericht.txt")));
    if(tf.open(QIODevice::WriteOnly|QIODevice::Text)) { tf.write(t.toUtf8()); tf.commit(); }
    return dir.filePath(QStringLiteral("bericht.txt"));
}

} // namespace RescueImager

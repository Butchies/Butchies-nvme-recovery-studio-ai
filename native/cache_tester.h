#pragma once
// Cache diagnosis of NVMe SSDs.
//  1. Cache equipment: DRAM vs. DRAM-less (HMB request), volatile write
//     cache present/enabled, HMB actually granted by the host (Linux).
//  2. NVMe device self-test (short/extended) executed by the controller,
//     result and failing segment from log page 0x06.
//  3. Read test (non-destructive): repeated vs. random 4 KiB latency,
//     jitter, sequential throughput.
//  4. Write test (Linux expert mode only, OVERWRITES DATA): write-cache /
//     SLC-cache behaviour, performance cliff, flush time.
// Indicators are evidence, not proof: the DRAM itself is not accessible.
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QVariantList>
#include <QElapsedTimer>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRandomGenerator>
#include <QThread>
#include <atomic>
#include <functional>
#include <vector>
#include <algorithm>
#include "platform_io.h"
#include "linux_nvme.h"

namespace CacheTester {

using Progress=std::function<void(int,const QString&)>;

inline double percentile(std::vector<double> v, double p) {
    if(v.empty()) return 0;
    std::sort(v.begin(),v.end());
    return v[size_t(std::min<double>(v.size()-1,p*(v.size()-1)))];
}

// --- 1 + 2: equipment and self-test ------------------------------------------
inline QVariantMap equipmentAndSelfTest(const QVariantMap &d, bool extended, const QString &smartctl,
                                        std::atomic_bool &cancel, const Progress &progress) {
    QVariantMap r;
#ifndef Q_OS_WIN
    const QString ctrl=d.value("controllerPath").toString();
    const int fd=ctrl.isEmpty() ? -1 : ::open(QFile::encodeName(ctrl).constData(),O_RDONLY|O_CLOEXEC);
    if(fd<0) { r["selbsttest"]=QStringLiteral("Controller nicht zugreifbar"); return r; }
    std::vector<unsigned char> id(4096,0);
    if(LinuxNvme::admin(fd,0x06,0,id.data(),4096,1)==0) {
        const auto u32=[&](int o){ return quint32(id[o])|quint32(id[o+1])<<8|quint32(id[o+2])<<16|quint32(id[o+3])<<24; };
        const quint32 hmpre=u32(272), hmmin=u32(276);
        r["hmbBevorzugtMiB"]=double(hmpre)*4096/1048576.0;
        r["hmbMinimumMiB"]=double(hmmin)*4096/1048576.0;
        r["dramlos"]=hmpre>0;
        r["schreibcacheVorhanden"]=(id[525]&0x1)!=0;
        r["selbsttestUnterstuetzt"]=(id[256]&0x10)!=0;
    }
    quint32 res=0;
    if(r.value("schreibcacheVorhanden").toBool() && LinuxNvme::admin(fd,0x0A,0,nullptr,0,0x06,0,5000,&res)==0)
        r["schreibcacheAktiv"]=(res&0x1)!=0;
    std::vector<unsigned char> hmb(4096,0);
    if(r.value("dramlos").toBool() && LinuxNvme::admin(fd,0x0A,0,hmb.data(),4096,0x0D,0,5000,&res)==0) {
        r["hmbAktiv"]=(res&0x1)!=0;
        const quint32 hsize=quint32(hmb[0])|quint32(hmb[1])<<8|quint32(hmb[2])<<16|quint32(hmb[3])<<24;
        r["hmbZugeteiltMiB"]=double(hsize)*4096/1048576.0;
    }
    if(r.value("selbsttestUnterstuetzt").toBool()) {
        progress(0,QStringLiteral("Controller-Selbsttest läuft …"));
        if(LinuxNvme::admin(fd,0x14,0xffffffff,nullptr,0,extended ? 2 : 1)!=0) r["selbsttest"]=QStringLiteral("Start abgelehnt");
        else {
            unsigned char log[564]{};
            for(int i=0;i<(extended ? 2880 : 120) && !cancel.load();++i) {
                QThread::sleep(5);
                if(LinuxNvme::getLog(fd,0x06,log,sizeof(log))!=0) break;
                if(log[0]==0) break;
                progress(log[1],QStringLiteral("Controller-Selbsttest läuft …"));
            }
            const unsigned status=log[4]&0xf;
            static const char *texts[]={"bestanden","abgebrochen (Befehl)","abgebrochen (Reset)","abgebrochen (Namespace entfernt)",
                "abgebrochen (Format)","fehlgeschlagen (schwerer Fehler)","fehlgeschlagen (unbekanntes Segment)",
                "fehlgeschlagen (Segment)","abgebrochen (unbekannt)","abgebrochen (Sanitize)"};
            r["selbsttest"]=status<10 ? QString::fromUtf8(texts[status]) : QStringLiteral("unbekannt (%1)").arg(status);
            r["selbsttestFehlgeschlagen"]=(status>=5 && status<=7);
            if(status==7) r["selbsttestSegment"]=int(log[5]);
        }
    }
    ::close(fd);
#else
    if(smartctl.isEmpty()) { r["selbsttest"]=QStringLiteral("smartctl nicht installiert"); return r; }
    const QString dev=QStringLiteral("/dev/pd%1").arg(d.value("devicePath").toString().mid(QStringLiteral("\\\\.\\PHYSICALDRIVE").size()));
    QStringList base{QStringLiteral("-d"),d.value("smartctlTyp",QStringLiteral("auto")).toString()};
    QProcess p;
    p.start(smartctl,base+QStringList{QStringLiteral("-t"),extended ? QStringLiteral("long") : QStringLiteral("short"),dev});
    p.waitForFinished(30000);
    progress(0,QStringLiteral("Controller-Selbsttest läuft …"));
    for(int i=0;i<(extended ? 2880 : 120) && !cancel.load();++i) {
        QThread::sleep(5);
        QProcess q; q.start(smartctl,base+QStringList{QStringLiteral("-l"),QStringLiteral("selftest"),QStringLiteral("-j"),dev});
        q.waitForFinished(30000);
        const auto log=QJsonDocument::fromJson(q.readAllStandardOutput()).object().value("nvme_self_test_log").toObject();
        if(log.isEmpty()) { r["selbsttest"]=QStringLiteral("über diese USB-Bridge nicht verfügbar"); break; }
        if(log.value("current_self_test_operation").toObject().value("value").toInt()==0) {
            const auto first=log.value("table").toArray().at(0).toObject().value("self_test_result").toObject();
            r["selbsttest"]=first.value("string").toString();
            const int v=first.value("value").toInt();
            r["selbsttestFehlgeschlagen"]=(v>=5 && v<=7);
            break;
        }
        progress(log.value("current_self_test_completion_percent").toInt(),QStringLiteral("Controller-Selbsttest läuft …"));
    }
#endif
    return r;
}

// --- 3: read test (non-destructive) --------------------------------------------
inline QVariantMap readTest(const QString &path, std::atomic_bool &cancel, const Progress &progress) {
    QVariantMap r;
    PlatformIO::SourceDevice src;
    QString err;
    if(!src.open(path,err)) { r["fehler"]=err; return r; }
    const quint32 block=qMax<quint32>(4096,src.sectorSize());
    void *buf=PlatformIO::allocAligned(1<<20,src.sectorSize());
    std::vector<double> repeat, random;
    int timeouts=0;
    const auto timed=[&](quint64 off,quint32 len)->double {
        QElapsedTimer t; t.start();
        const auto res=src.read(off,buf,len,5000);
        if(res.status!=PlatformIO::ReadStatus::Ok) { ++timeouts; return -1; }
        return t.nsecsElapsed()/1000.0;
    };
    const quint64 blocks=src.size()/block;
    for(int i=0;i<300 && !cancel.load();++i) { const double us=timed(0,block); if(us>=0 && i>=10) repeat.push_back(us); if(i%30==0) progress(i*30/300,QStringLiteral("Lesetest: wiederholte Zugriffe …")); }
    for(int i=0;i<600 && !cancel.load();++i) {
        const double us=timed(QRandomGenerator::global()->bounded(blocks)*block,block);
        if(us>=0) random.push_back(us);
        if(i%30==0) progress(30+i*40/600,QStringLiteral("Lesetest: Zufallszugriffe …"));
    }
    QElapsedTimer seq; seq.start(); quint64 bytes=0;
    for(quint64 off=0;off<qMin<quint64>(src.size(),512ULL<<20) && !cancel.load();off+=1<<20) {
        if(src.read(off,buf,1<<20,10000).status!=PlatformIO::ReadStatus::Ok) { ++timeouts; break; }
        bytes+=1<<20;
        if((off>>20)%32==0) progress(70+int(30*off/(512ULL<<20)),QStringLiteral("Lesetest: sequenziell …"));
    }
    if(!src.hung()) PlatformIO::freeAligned(buf);
    const double rep50=percentile(repeat,0.5), rnd50=percentile(random,0.5), rnd99=percentile(random,0.99);
    r["wiederholtMedianUs"]=rep50;
    r["zufallMedianUs"]=rnd50;
    r["zufallP99Us"]=rnd99;
    r["sequenziellMiBs"]=seq.elapsed()>0 ? bytes/1048576.0/(seq.elapsed()/1000.0) : 0;
    r["zeitueberschreitungen"]=timeouts;
    // Indicators: a working cache makes repeated reads clearly faster than
    // random ones; very large outliers point to unstable controller/cache.
    r["cacheNichtWirksam"]=rep50>0 && rnd50>0 && rep50>0.8*rnd50 && rnd50>150;
    r["latenzInstabil"]=rnd50>0 && rnd99/rnd50>25;
    return r;
}

// --- 4: write test (Linux, expert mode, destroys data in the tested area) --------
inline QVariantMap writeTest(const QVariantMap &d, std::atomic_bool &cancel, const Progress &progress) {
    QVariantMap r;
#ifdef Q_OS_WIN
    Q_UNUSED(d) Q_UNUSED(cancel) Q_UNUSED(progress)
    r["fehler"]=QStringLiteral("Schreibtest nur in der Linux-Boot-Edition.");
#else
    if(d.value("protected").toBool() || !d.value("mounted").toStringList().isEmpty()) { r["fehler"]=QStringLiteral("Laufwerk geschützt oder eingebunden."); return r; }
    const QString path=d.value("devicePath").toString();
    const int fd=::open(QFile::encodeName(path).constData(),O_RDWR|O_DIRECT|O_CLOEXEC);
    if(fd<0) { r["fehler"]=QStringLiteral("Nicht beschreibbar (%1)").arg(QString::fromLocal8Bit(strerror(errno))); return r; }
    unsigned long long size=0; ioctl(fd,BLKGETSIZE64,&size);
    const quint64 total=qMin<quint64>(32ULL<<30,size/4);
    const quint32 chunk=1<<20;
    void *buf=PlatformIO::allocAligned(chunk,4096);
    for(quint32 i=0;i<chunk;++i) static_cast<unsigned char*>(buf)[i]=static_cast<unsigned char>(QRandomGenerator::global()->generate());
    QVariantList slices;
    double first=0, minRate=1e12; quint64 cliffAt=0;
    QElapsedTimer slice; slice.start();
    quint64 written=0;
    for(quint64 off=0;off<total && !cancel.load();off+=chunk) {
        if(pwrite(fd,buf,chunk,off_t(off))!=ssize_t(chunk)) { r["fehler"]=QStringLiteral("Schreibfehler bei Byte %1").arg(off); break; }
        written+=chunk;
        if(written%(256ULL<<20)==0) {
            const double rate=256.0/(slice.nsecsElapsed()/1e9);
            slice.restart();
            slices.append(rate);
            if(first==0) first=rate;
            if(!cliffAt && rate<0.5*first) cliffAt=written;
            minRate=std::min(minRate,rate);
            progress(int(100*written/total),QStringLiteral("Schreibtest (überschreibt Daten) …"));
        }
    }
    QElapsedTimer flush; flush.start();
    fdatasync(fd);
    r["flushMs"]=flush.elapsed();
    ::close(fd);
    PlatformIO::freeAligned(buf);
    r["geschriebenGiB"]=written/1073741824.0;
    r["startMiBs"]=first;
    r["minimumMiBs"]=minRate>1e11 ? 0 : minRate;
    r["einbruchNachGiB"]=cliffAt ? cliffAt/1073741824.0 : -1.0;
    r["verlauf"]=slices;
    r["schreibcacheEinbruch"]=cliffAt>0;
    r["flushLangsam"]=r.value("flushMs").toLongLong()>5000;
#endif
    return r;
}

// Summary text and indicators for the agent.
inline QVariantMap evaluate(const QVariantMap &eq, const QVariantMap &rd, const QVariantMap &wr) {
    QStringList lines; bool defect=false;
    if(eq.contains("dramlos"))
        lines<<(eq.value("dramlos").toBool()
            ? QStringLiteral("DRAM-los (nutzt Host-Speicher): verlangt %1 MiB, zugeteilt %2 MiB%3.")
                .arg(eq.value("hmbBevorzugtMiB").toDouble(),0,'f',0).arg(eq.value("hmbZugeteiltMiB").toDouble(),0,'f',0)
                .arg(eq.value("hmbAktiv").toBool() ? QString() : QStringLiteral(" – HMB nicht aktiv, Cache wird NICHT angesprochen"))
            : QStringLiteral("Eigener DRAM-Cache (kein Host-Speicher angefordert)."));
    if(eq.contains("schreibcacheVorhanden"))
        lines<<QStringLiteral("Flüchtiger Schreibcache: %1%2.").arg(eq.value("schreibcacheVorhanden").toBool() ? QStringLiteral("vorhanden") : QStringLiteral("nicht gemeldet"),
            eq.contains("schreibcacheAktiv") ? (eq.value("schreibcacheAktiv").toBool() ? QStringLiteral(", aktiv") : QStringLiteral(", ABGESCHALTET")) : QString());
    if(eq.contains("selbsttest")) lines<<QStringLiteral("Controller-Selbsttest: %1%2.").arg(eq.value("selbsttest").toString(),
        eq.contains("selbsttestSegment") ? QStringLiteral(" (Segment %1)").arg(eq.value("selbsttestSegment").toInt()) : QString());
    if(rd.contains("zufallMedianUs"))
        lines<<QStringLiteral("Lesen: wiederholt %1 µs, zufällig %2 µs (99 %: %3 µs), sequenziell %4 MiB/s.")
            .arg(rd.value("wiederholtMedianUs").toDouble(),0,'f',0).arg(rd.value("zufallMedianUs").toDouble(),0,'f',0)
            .arg(rd.value("zufallP99Us").toDouble(),0,'f',0).arg(rd.value("sequenziellMiBs").toDouble(),0,'f',0);
    if(wr.contains("startMiBs"))
        lines<<QStringLiteral("Schreiben: Start %1 MiB/s, Minimum %2 MiB/s%3, Flush %4 ms.")
            .arg(wr.value("startMiBs").toDouble(),0,'f',0).arg(wr.value("minimumMiBs").toDouble(),0,'f',0)
            .arg(wr.value("einbruchNachGiB").toDouble()>0 ? QStringLiteral(", Einbruch nach %1 GiB (Cache voll)").arg(wr.value("einbruchNachGiB").toDouble(),0,'f',1) : QString())
            .arg(wr.value("flushMs").toLongLong());
    const bool hmbProblem=eq.value("dramlos").toBool() && eq.contains("hmbAktiv") && !eq.value("hmbAktiv").toBool();
    const bool wcOff=eq.contains("schreibcacheAktiv") && !eq.value("schreibcacheAktiv").toBool();
    defect=eq.value("selbsttestFehlgeschlagen").toBool() || rd.value("cacheNichtWirksam").toBool() || rd.value("latenzInstabil").toBool()
        || wr.value("flushLangsam").toBool() || hmbProblem;
    QString verdict;
    if(eq.value("selbsttestFehlgeschlagen").toBool()) verdict=QStringLiteral("Controller-Selbsttest fehlgeschlagen – Hinweis auf Defekt in Controller/Cache/Flash.");
    else if(hmbProblem) verdict=QStringLiteral("Cache wird nicht angesprochen: Die DRAM-lose SSD erhält keinen Host-Speicher (Treiber/Betriebssystem oder Controller).");
    else if(rd.value("cacheNichtWirksam").toBool()) verdict=QStringLiteral("Lese-Cache zeigt keine Wirkung – Hinweis auf nicht angesprochenen oder defekten Cache.");
    else if(rd.value("latenzInstabil").toBool()) verdict=QStringLiteral("Stark schwankende Antwortzeiten – Controller oder Cache instabil.");
    else if(wr.value("flushLangsam").toBool()) verdict=QStringLiteral("Cache-Leerung (Flush) sehr langsam – Schreibcache oder Flash-Verwaltung auffällig.");
    else if(wcOff) verdict=QStringLiteral("Schreibcache ist abgeschaltet – Leistung reduziert, aber kein Defektnachweis.");
    else verdict=QStringLiteral("Keine Auffälligkeiten am Cache erkennbar.");
    return {{"urteil",verdict},{"auffaellig",defect},{"details",lines},
            {"selbsttestFehlgeschlagen",eq.value("selbsttestFehlgeschlagen").toBool()},
            {"cacheNichtWirksam",rd.value("cacheNichtWirksam").toBool() || hmbProblem},
            {"latenzInstabil",rd.value("latenzInstabil").toBool()},
            {"ausstattung",eq},{"lesen",rd},{"schreiben",wr}};
}

} // namespace CacheTester

#pragma once
// Automatic repair chain for defective NVMe SSDs (Linux Boot Edition, data
// on the SSD is NOT preserved). From gentle to hard; after every step the
// SSD is checked and the chain stops as soon as it works again:
//   A  PCIe level  (controller without NVMe function / not responding):
//      A1 PCIe function reset, A2 remove + bus rescan
//   B  Controller responds:
//      B1 controller reset
//      B2 activate another firmware slot with a different revision
//      B3 Format NVM with user-data erase (rebuilds the mapping table)
//      B4 Sanitize Block Erase (erases all flash incl. caches)
//      B5 reflash firmware (only if a file was provided)
//   C  Verification: controller answers, capacity > 0, no critical SMART
//      warning, surface test (write pattern + read back + compare).
// Every step is logged with the controller's translated answer.
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QVariantList>
#include <QFile>
#include <QDir>
#include <QThread>
#include <QElapsedTimer>
#include <atomic>
#include <functional>
#include <vector>
#include <thread>
#include <QMutex>
#include "linux_nvme.h"
#include "error_translator.h"
#include "firmware_manager.h"

namespace RepairChain {

using Log=std::function<void(const QString&)>;
using Progress=std::function<void(int,const QString&)>;

struct Options { bool fullSurface=false; QString firmwareFile; };

#ifndef Q_OS_WIN
inline bool writeSys(const QString &path, const QByteArray &v) {
    QFile f(path); return f.open(QIODevice::WriteOnly) && f.write(v)==v.size();
}

// Finds the device again after resets (same PCI address).
inline QVariantMap refind(const QString &pciAddress, const QSet<QString> &vendors) {
    for(const auto &d:LinuxNvme::enumerate(vendors)) if(d.toMap().value("pciAddress").toString()==pciAddress) return d.toMap();
    return {};
}

// Pattern surface test: each 4 KiB block holds its own offset; written and
// read back with O_DIRECT. full=false tests 3 × 1 GiB (start/middle/end).
inline QVariantMap surfaceTest(const QString &dev, bool full, std::atomic_bool &cancel, const Progress &progress) {
    QVariantMap r;
    const int fd=::open(QFile::encodeName(dev).constData(),O_RDWR|O_DIRECT|O_CLOEXEC);
    if(fd<0) { r["ok"]=false; r["fehler"]=QStringLiteral("%1 nicht beschreibbar").arg(dev); return r; }
    unsigned long long size=0; ioctl(fd,BLKGETSIZE64,&size);
    if(size==0) { ::close(fd); r["ok"]=false; r["fehler"]=QStringLiteral("Kapazität 0"); return r; }
    const quint64 chunk=4ULL<<20, gib=1ULL<<30;
    std::vector<std::pair<quint64,quint64>> ranges;
    if(full || size<=3*gib) ranges.push_back({0,size});
    else { ranges.push_back({0,gib}); ranges.push_back({(size/2)&~(chunk-1),gib}); ranges.push_back({(size-gib)&~(chunk-1),gib}); }
    quint64 total=0; for(auto &x:ranges) total+=x.second;
    auto *w=static_cast<quint64*>(PlatformIO::allocAligned(chunk,4096));
    auto *rd=static_cast<quint64*>(PlatformIO::allocAligned(chunk,4096));
    quint64 done=0, bad=0, errors=0; int last=-1;
    for(int pass=0;pass<2 && !cancel.load();++pass) {
        for(const auto &rg:ranges) {
            for(quint64 off=rg.first;off<rg.first+rg.second && !cancel.load();off+=chunk) {
                const quint64 len=qMin(chunk,rg.first+rg.second-off);
                for(quint64 i=0;i<len/8;++i) w[i]=(off+i*8)^0x5A5AA5A5C3C33C3CULL;
                if(pass==0) { if(pwrite(fd,w,len,off_t(off))!=ssize_t(len)) ++errors; }
                else {
                    if(pread(fd,rd,len,off_t(off))!=ssize_t(len)) { ++errors; continue; }
                    if(memcmp(w,rd,len)!=0) ++bad;
                }
                done+=len;
                const int pct=int(100*done/(2*total));
                if(pct!=last) { last=pct; progress(pct,pass==0 ? QStringLiteral("Oberflächentest: schreiben …") : QStringLiteral("Oberflächentest: prüfen …")); }
            }
            if(pass==0) fdatasync(fd);
        }
    }
    ::close(fd);
    PlatformIO::freeAligned(w); PlatformIO::freeAligned(rd);
    r["ok"]=!cancel.load() && bad==0 && errors==0;
    r["getestetGiB"]=total/1073741824.0;
    r["abweichendeBloecke"]=QString::number(bad);
    r["ioFehler"]=QString::number(errors);
    r["vollstaendig"]=full || size<=3*gib;
    return r;
}

// Zero-fill test: writes zeros over the whole capacity until full, then reads
// everything back and checks every byte is zero. Tests all flash chips.
inline QVariantMap zeroFillTest(const QString &dev, std::atomic_bool &cancel, const Progress &progress) {
    // Parallel version: several threads with their own file descriptors keep
    // many large requests in flight, so the SSD works on all flash chips at
    // once (full test, same thoroughness, close to the SSD's top speed).
    QVariantMap r;
    int probe=::open(QFile::encodeName(dev).constData(),O_RDWR|O_DIRECT|O_CLOEXEC);
    if(probe<0) { r["ok"]=false; r["fehler"]=QStringLiteral("%1 nicht beschreibbar").arg(dev); return r; }
    unsigned long long size=0; ioctl(probe,BLKGETSIZE64,&size); ::close(probe);
    const quint64 chunk=16ULL<<20;
    const int threads=8;
    const quint64 chunks=(size+chunk-1)/chunk;
    std::atomic<quint64> done{0}, writeErr{0}, readErr{0}, badChunks{0};
    QMutex badMutex; QVariantList badAreas;
    QElapsedTimer timer; timer.start();
    for(int pass=0;pass<2 && !cancel.load();++pass) {
        std::vector<std::thread> pool;
        std::atomic<int> finished{0};
        for(int t=0;t<threads;++t) pool.emplace_back([&,t,pass]{
            const int fd=::open(QFile::encodeName(dev).constData(),O_RDWR|O_DIRECT|O_CLOEXEC);
            if(fd<0) { writeErr++; finished++; return; }
            auto *buf=static_cast<unsigned char*>(PlatformIO::allocAligned(chunk,4096));
            if(pass==0) memset(buf,0,chunk);
            for(quint64 c=quint64(t);c<chunks && !cancel.load();c+=quint64(threads)) {
                const quint64 off=c*chunk, len=qMin(chunk,size-off);
                if(pass==0) { if(pwrite(fd,buf,len,off_t(off))!=ssize_t(len)) writeErr++; }
                else if(pread(fd,buf,len,off_t(off))!=ssize_t(len)) {
                    readErr++; QMutexLocker l(&badMutex); if(badAreas.size()<100) badAreas.append(QStringLiteral("Lesefehler ab Byte %1").arg(off));
                } else {
                    const quint64 *w=reinterpret_cast<const quint64*>(buf); bool zero=true;
                    for(quint64 i=0;i<len/8;++i) if(w[i]) { zero=false; break; }
                    if(!zero) { badChunks++; QMutexLocker l(&badMutex); if(badAreas.size()<100) badAreas.append(QStringLiteral("Nicht null ab Byte %1 (16 MiB-Block)").arg(off)); }
                }
                done+=len;
            }
            if(pass==0) fdatasync(fd);
            ::close(fd);
            PlatformIO::freeAligned(buf);
            finished++;
        });
        int last=-1;
        while(true) {
            const int pct=int(done.load()*100/(2*size));
            if(pct!=last) {
                last=pct;
                const double secs=timer.elapsed()/1000.0, rate=secs>0 ? done.load()/1048576.0/secs : 0;
                progress(pct,QStringLiteral("%1 – %2 MiB/s").arg(pass==0 ? QStringLiteral("Nullfüll-Test: beschreibe SSD bis voll") : QStringLiteral("Nullfüll-Test: prüfe alle Bereiche")).arg(rate,0,'f',0));
            }
            if(finished.load()>=threads) break;
            QThread::msleep(500);
        }
        for(auto &th:pool) th.join();
    }
    r["ok"]=!cancel.load() && writeErr==0 && readErr==0 && badChunks==0;
    r["kapazitaetGiB"]=size/1073741824.0;
    r["dauerMinuten"]=timer.elapsed()/60000.0;
    r["schreibfehler"]=QString::number(writeErr.load());
    r["lesefehler"]=QString::number(readErr.load());
    r["fehlerhafteBloecke"]=QString::number(badChunks.load());
    r["fehlerbereiche"]=badAreas;
    return r;
}

// Quick health check after a step.
inline QVariantMap check(const QVariantMap &d, std::atomic_bool &cancel, const Progress &progress) {
    QVariantMap r;
    if(d.isEmpty() || d.value("romModeSuspect").toBool() || d.value("devicePath").toString().isEmpty()) {
        r["ok"]=false; r["grund"]=QStringLiteral("Controller ohne NVMe-Funktion bzw. ohne Speicherbereich"); return r;
    }
    const auto in=LinuxNvme::inspect(d);
    if(!in.value("nvmeIdentifyAvailable").toBool()) { r["ok"]=false; r["grund"]=QStringLiteral("Controller antwortet nicht auf Identify"); return r; }
    if(d.value("sizeBytes").toString().toULongLong()==0) { r["ok"]=false; r["grund"]=QStringLiteral("Kapazität 0"); return r; }
    const unsigned warn=in.value("criticalWarning").toString().toUInt(nullptr,16);
    if(warn & 0x0c) { r["ok"]=false; r["grund"]=QStringLiteral("SMART meldet Zuverlässigkeits-/Nur-Lese-Zustand"); return r; }
    const auto s=surfaceTest(d.value("devicePath").toString(),false,cancel,progress);
    r["oberflaeche"]=s;
    r["ok"]=s.value("ok").toBool();
    if(!r["ok"].toBool()) r["grund"]=QStringLiteral("Oberflächentest: %1 abweichende Blöcke, %2 I/O-Fehler").arg(s.value("abweichendeBloecke").toString(),s.value("ioFehler").toString());
    return r;
}
#endif

inline QVariantMap run(QVariantMap d, const Options &opt, const QSet<QString> &vendors,
                       std::atomic_bool &cancel, const Log &log, const Progress &progress) {
    QVariantMap result; QVariantList steps;
#ifdef Q_OS_WIN
    Q_UNUSED(d) Q_UNUSED(opt) Q_UNUSED(vendors) Q_UNUSED(cancel) Q_UNUSED(progress)
    log(QStringLiteral("Reparaturkette nur in der Linux-Boot-Edition (direkter PCIe-Zugriff). Unter Windows über USB reichen Gehäuse Format/Sanitize nicht durch."));
    result["ok"]=false;
    result["message"]=QStringLiteral("Nur in der Linux-Boot-Edition verfügbar.");
    return result;
#else
    if(d.value("protected").toBool()) { result["message"]=QStringLiteral("Boot-Stick/Systemlaufwerk – wird nicht verändert."); return result; }
    const QString pci=d.value("pciAddress").toString();
    const QString base=QStringLiteral("/sys/bus/pci/devices/")+pci;
    const auto step=[&](const QString &name,const QString &answer,bool ok) {
        steps.append(QVariantMap{{"schritt",name},{"antwort",answer},{"ok",ok}});
        log(QStringLiteral("%1 %2: %3").arg(ok ? QStringLiteral("✓") : QStringLiteral("✗"),name,answer));
    };
    const auto verify=[&](const QString &after)->bool {
        QThread::sleep(3);
        d=refind(pci,vendors);
        const auto c=check(d,cancel,progress);
        log(QStringLiteral("Prüfung nach „%1“: %2").arg(after,c.value("ok").toBool() ? QStringLiteral("SSD arbeitet wieder einwandfrei") : c.value("grund").toString()));
        if(c.value("ok").toBool()) { result["repariertDurch"]=after; return true; }
        return false;
    };
    const auto translate=[](int st){ return st==0 ? QStringLiteral("erfolgreich") : st<0 ? QStringLiteral("Systemfehler: %1").arg(QString::fromLocal8Bit(strerror(-st)))
        : ErrorTranslator::status(unsigned(st)).value("uebersetzung").toString()+QStringLiteral(" (")+ErrorTranslator::status(unsigned(st)).value("roh").toString()+QStringLiteral(")"); };

    // --- A: PCIe level --------------------------------------------------------
    if(d.value("controllerPath").toString().isEmpty() || d.value("romModeSuspect").toBool()) {
        const bool r1=writeSys(base+QStringLiteral("/reset"),"1");
        step(QStringLiteral("PCIe-Funktionsreset"),r1 ? QStringLiteral("ausgeführt") : QStringLiteral("vom Gerät/Steckplatz nicht unterstützt"),r1);
        if(r1 && verify(QStringLiteral("PCIe-Funktionsreset"))) goto done;
        if(cancel.load()) goto done;
        {
            const bool r2=writeSys(base+QStringLiteral("/remove"),"1");
            QThread::sleep(2);
            const bool r3=writeSys(QStringLiteral("/sys/bus/pci/rescan"),"1");
            step(QStringLiteral("PCIe entfernen und neu einlesen"),(r2&&r3) ? QStringLiteral("ausgeführt") : QStringLiteral("fehlgeschlagen"),r2&&r3);
            QThread::sleep(5);
            if(verify(QStringLiteral("PCIe neu einlesen"))) goto done;
        }
        if(d.value("controllerPath").toString().isEmpty()) {
            step(QStringLiteral("Notmodus"),QStringLiteral("Controller bleibt ohne NVMe-Funktion. Weiter nur mit Herstellerwerkzeug (MPTool) der Controllerfamilie oder Labor."),false);
            result["naechsterWeg"]=QStringLiteral("notmodus_hersteller_werkzeug");
            goto done;
        }
    }

    // --- B: controller responds --------------------------------------------------
    {
        const QString ctrl=d.value("controllerPath").toString();
        const int fd=::open(QFile::encodeName(ctrl).constData(),O_RDWR|O_CLOEXEC);
        if(fd<0) { step(QStringLiteral("Controller öffnen"),QString::fromLocal8Bit(strerror(errno)),false); goto done; }
        const auto in=LinuxNvme::inspect(d);

        // B1 controller reset
        {
            const int rr=ioctl(fd,NVME_IOCTL_RESET);
            step(QStringLiteral("Controller-Reset"),rr==0 ? QStringLiteral("ausgeführt") : QString::fromLocal8Bit(strerror(errno)),rr==0);
            if(verify(QStringLiteral("Controller-Reset"))) { ::close(fd); goto done; }
        }
        // B2 other firmware slot
        if(!cancel.load()) {
            const auto revs=in.value("firmwareSlotRevisions").toList();
            const int active=in.value("firmwareActiveSlot").toInt();
            const QString cur=in.value("nvmeFirmware").toString();
            int slot=0;
            for(int i=0;i<revs.size();++i) { const QString r=revs[i].toString(); if(i+1!=active && r!=QStringLiteral("leer") && r!=cur) { slot=i+1; break; } }
            if(slot) {
                const int st=LinuxNvme::admin(fd,0x10,0,nullptr,0,quint32(2<<3)|quint32(slot),0,120000);
                const int sc=st>0 ? (st&0x7ff) : st;
                const bool ok=st==0 || sc==0x10b || sc==0x110 || sc==0x111;
                step(QStringLiteral("Firmware-Slot %1 (%2) aktivieren").arg(slot).arg(revs[slot-1].toString()),translate(st),ok);
                if(ok) { ioctl(fd,NVME_IOCTL_RESET); if(verify(QStringLiteral("Firmware-Slot wechseln"))) { ::close(fd); goto done; } }
            } else step(QStringLiteral("Firmware-Slot wechseln"),QStringLiteral("keine andere Firmware-Version in den Slots vorhanden"),false);
        }
        // B3 Format NVM with user data erase
        if(!cancel.load() && (in.value("oacs").toUInt()&0x2)) {
            unsigned char ns[4096]{};
            const int idn=LinuxNvme::admin(fd,0x06,1,ns,4096,0);
            const quint32 lbaf=idn==0 ? (ns[26]&0x0f) : 0;
            progress(0,QStringLiteral("Format NVM läuft (kann Minuten dauern) …"));
            int st=LinuxNvme::admin(fd,0x80,0xffffffff,nullptr,0,lbaf|(1u<<9),0,1800000);
            if(st!=0) st=LinuxNvme::admin(fd,0x80,1,nullptr,0,lbaf|(1u<<9),0,1800000);
            step(QStringLiteral("Format NVM mit Löschen der Nutzerdaten (Verwaltungstabelle neu aufbauen)"),translate(st),st==0);
            if(st==0) { ioctl(fd,NVME_IOCTL_RESET); if(verify(QStringLiteral("Format NVM"))) { ::close(fd); goto done; } }
        }
        // B4 Sanitize Block Erase
        if(!cancel.load()) {
            std::vector<unsigned char> id(4096,0);
            LinuxNvme::admin(fd,0x06,0,id.data(),4096,1);
            const bool blockErase=(id[328]&0x2)!=0;
            if(blockErase) {
                const int st=LinuxNvme::admin(fd,0x84,0,nullptr,0,2,0,60000);
                step(QStringLiteral("Sanitize Block Erase gestartet (löscht gesamten Flash inkl. Caches)"),translate(st),st==0);
                if(st==0) {
                    unsigned char log[512]{};
                    for(int i=0;i<720 && !cancel.load();++i) {
                        QThread::sleep(5);
                        if(LinuxNvme::getLog(fd,0x81,log,512)!=0) continue;
                        const unsigned sprog=unsigned(log[0])|(unsigned(log[1])<<8);
                        const unsigned sstat=log[2]&0x7;
                        progress(int(sprog*100/65535),QStringLiteral("Sanitize läuft …"));
                        if(sstat==1 || sstat==3) {
                            step(QStringLiteral("Sanitize-Ergebnis"),sstat==1 ? QStringLiteral("abgeschlossen") : QStringLiteral("FEHLGESCHLAGEN – Flash/Controller defekt"),sstat==1);
                            break;
                        }
                    }
                    ioctl(fd,NVME_IOCTL_RESET);
                    if(verify(QStringLiteral("Sanitize Block Erase"))) { ::close(fd); goto done; }
                }
            } else step(QStringLiteral("Sanitize Block Erase"),QStringLiteral("vom Controller nicht unterstützt"),false);
        }
        ::close(fd);
        // B5 reflash firmware
        if(!cancel.load() && !opt.firmwareFile.isEmpty()) {
            const auto fresh=LinuxNvme::inspect(d);
            QVariantMap dev=d; for(auto it=fresh.begin();it!=fresh.end();++it) dev[it.key()]=it.value();
            const auto plan=FirmwareManager::prepare(dev,{},opt.firmwareFile,{},true,true);
            if(plan.value("allowed").toBool()) {
                const auto fr=FirmwareManager::execute(plan,dev,QString(),[&](int p,const QString &t){ progress(p,t); });
                step(QStringLiteral("Firmware neu aufspielen"),fr.value("message").toString(),fr.value("ok").toBool());
                if(verify(QStringLiteral("Firmware neu aufspielen"))) goto done;
            } else step(QStringLiteral("Firmware neu aufspielen"),plan.value("summary").toString(),false);
        }
        result["naechsterWeg"]=QStringLiteral("notmodus_hersteller_werkzeug_oder_labor");
    }

done:
    if(result.contains("repariertDurch") && !cancel.load()) {
        log(QStringLiteral("Abschluss-Oberflächentest (%1) …").arg(opt.fullSurface ? QStringLiteral("gesamte SSD") : QStringLiteral("Stichprobe 3 × 1 GiB")));
        const auto s=surfaceTest(d.value("devicePath").toString(),opt.fullSurface,cancel,progress);
        result["abschlusstest"]=s;
        step(QStringLiteral("Abschluss-Oberflächentest"),QStringLiteral("%1 GiB, %2 abweichende Blöcke, %3 I/O-Fehler").arg(s.value("getestetGiB").toDouble(),0,'f',1)
             .arg(s.value("abweichendeBloecke").toString(),s.value("ioFehler").toString()),s.value("ok").toBool());
        result["ok"]=s.value("ok").toBool();
        // Mandatory after a successful repair: fill the whole SSD with zeros and verify.
        if(result.value("ok").toBool() && !cancel.load()) {
            log(QStringLiteral("Nullfüll-Test: SSD wird komplett mit 0 beschrieben und vollständig geprüft …"));
            const auto z=zeroFillTest(d.value("devicePath").toString(),cancel,progress);
            result["nullfuelltest"]=z;
            step(QStringLiteral("Nullfüll-Test (Speicherchips)"),QStringLiteral("%1 GiB: %2 Schreibfehler, %3 Lesefehler, %4 fehlerhafte Blöcke")
                 .arg(z.value("kapazitaetGiB").toDouble(),0,'f',1).arg(z.value("schreibfehler").toString(),z.value("lesefehler").toString(),z.value("fehlerhafteBloecke").toString()),
                 z.value("ok").toBool());
            result["ok"]=z.value("ok").toBool();
            if(!z.value("ok").toBool()) result["naechsterWeg"]=QStringLiteral("speicherchips_defekt");
        }
    } else result["ok"]=false;
    result["schritte"]=steps;
    result["controllerMeldungen"]=ErrorTranslator::report(d);
    result["message"]=result.value("ok").toBool()
        ? QStringLiteral("REPARIERT durch „%1“ – SSD arbeitet wieder einwandfrei.").arg(result.value("repariertDurch").toString())
        : (cancel.load() ? QStringLiteral("Abgebrochen.") : QStringLiteral("Automatische Reparatur ohne Erfolg. Weitere Möglichkeiten: %1.").arg(
              result.value("naechsterWeg").toString()==QStringLiteral("speicherchips_defekt") ? QStringLiteral("Controller läuft wieder, aber der Nullfüll-Test fand defekte Speicherbereiche – Speicherchips beschädigt, SSD nicht mehr verlässlich")
            : result.value("naechsterWeg").toString()==QStringLiteral("notmodus_hersteller_werkzeug") ? QStringLiteral("Herstellerwerkzeug (MPTool) für den Notmodus oder Labor")
                                                                                                     : QStringLiteral("Herstellerwerkzeug oder Labor; Flash/Controller vermutlich defekt")));
    return result;
#endif
}

} // namespace RepairChain

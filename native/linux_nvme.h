#pragma once
// Linux Boot Edition: direct PCIe/NVMe access.
//  - enumerate(): NVMe controllers and namespaces from sysfs, plus PCI
//    devices of known SSD controller vendors without working NVMe function
//    (ROM/safe mode suspicion)
//  - protection of boot medium and system disks
//  - NVMe admin commands via NVME_IOCTL_ADMIN_CMD (Identify, logs, and –
//    only for the firmware manager after approval – firmware download/commit)
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QVariantList>
#include <QSet>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <functional>
#include <vector>
#include "platform_io.h"
#ifndef Q_OS_WIN
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/nvme_ioctl.h>
#endif

namespace LinuxNvme {

inline QString readSys(const QString &path) {
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? QString::fromUtf8(f.readAll()).trimmed() : QString();
}
inline QString hex4(const QString &v) { QString s=v; s.remove(QStringLiteral("0x")); return s.rightJustified(4,QLatin1Char('0')).toLower(); }

#ifndef Q_OS_WIN
// Disks that must never be touched: boot medium, root, /boot, persistence.
inline QSet<QString> protectedDisks(QStringList *reasons=nullptr) {
    QSet<QString> out;
    QFile f(QStringLiteral("/proc/self/mountinfo"));
    if(!f.open(QIODevice::ReadOnly)) return out;
    const QStringList critical{QStringLiteral("/"),QStringLiteral("/boot"),QStringLiteral("/boot/efi"),
        QStringLiteral("/run/live/medium"),QStringLiteral("/run/live/persistence"),QStringLiteral("/lib/live/mount/medium"),
        QStringLiteral("/usr")};
    for(const auto &line:QString::fromUtf8(f.readAll()).split(QLatin1Char('\n'))) {
        const auto parts=line.split(QLatin1Char(' '));
        if(parts.size()<5) continue;
        const QString mountPoint=parts[4];
        bool hit=critical.contains(mountPoint) || mountPoint.startsWith(QStringLiteral("/run/live/persistence/"));
        if(!hit) continue;
        const auto mm=parts[2].split(QLatin1Char(':'));
        if(mm.size()!=2) continue;
        const dev_t dev=makedev(mm[0].toUInt(),mm[1].toUInt());
        const auto disks=PlatformIO::diskNamesForDev(dev);
        for(const auto &d:disks) {
            out.insert(d);
            if(reasons) reasons->append(QStringLiteral("%1 (%2)").arg(d,mountPoint));
        }
    }
    return out;
}
// Mounted partitions of a disk (firmware writes require none).
inline QStringList mountedPartitions(const QString &disk) {
    QStringList out;
    QFile f(QStringLiteral("/proc/self/mountinfo"));
    if(!f.open(QIODevice::ReadOnly)) return out;
    for(const auto &line:QString::fromUtf8(f.readAll()).split(QLatin1Char('\n'))) {
        const auto parts=line.split(QLatin1Char(' '));
        if(parts.size()<5) continue;
        const auto mm=parts[2].split(QLatin1Char(':'));
        if(mm.size()!=2) continue;
        if(PlatformIO::diskNamesForDev(makedev(mm[0].toUInt(),mm[1].toUInt())).contains(disk)) out<<parts[4];
    }
    return out;
}
#endif

// Signature for plug/unplug detection.
inline QString signature() {
    return QDir(QStringLiteral("/sys/bus/pci/devices")).entryList(QDir::Dirs|QDir::NoDotAndDotDot).join(QLatin1Char(','))
         +QLatin1Char('|')+QDir(QStringLiteral("/sys/block")).entryList(QDir::Dirs|QDir::NoDotAndDotDot|QDir::System).join(QLatin1Char(','));
}

// vendorIds: PCI vendors of SSD controllers from the templates.
inline QVariantList enumerate(const QSet<QString> &vendorIds) {
    QVariantList list;
#ifndef Q_OS_WIN
    QStringList reasons;
    const auto prot=protectedDisks(&reasons);
    const QDir pci(QStringLiteral("/sys/bus/pci/devices"));
    for(const auto &addr:pci.entryList(QDir::Dirs|QDir::NoDotAndDotDot|QDir::System)) {
        const QString base=pci.filePath(addr);
        const QString cls=readSys(base+QStringLiteral("/class")).toLower();
        const QString vid=hex4(readSys(base+QStringLiteral("/vendor")));
        const QString did=hex4(readSys(base+QStringLiteral("/device")));
        const bool isNvmeClass=cls.startsWith(QStringLiteral("0x010802"));
        const bool knownVendor=vendorIds.contains(vid);
        if(!isNvmeClass && !(knownVendor && (cls.startsWith(QStringLiteral("0x01")) || cls.startsWith(QStringLiteral("0x00")) || cls.startsWith(QStringLiteral("0xff")))))
            continue;
        const QString driver=QFileInfo(base+QStringLiteral("/driver")).symLinkTarget().section(QLatin1Char('/'),-1);
        QVariantMap common;
        common["edition"]="linux";
        common["connection"]="PCIe";
        common["storageBus"]="PCIe";
        common["readOnly"]=true;
        common["pciAddress"]=addr;
        common["pciVendorId"]=vid;
        common["pciDeviceId"]=did;
        common["pciClass"]=cls;
        common["pciSubsystem"]=QStringLiteral("%1:%2").arg(hex4(readSys(base+QStringLiteral("/subsystem_vendor"))),hex4(readSys(base+QStringLiteral("/subsystem_device"))));
        common["driver"]=driver.isEmpty() ? QStringLiteral("kein Treiber") : driver;
        common["bridgeVendorId"]=QStringLiteral("PCIe");
        common["bridgeProductId"]=addr;
        const QStringList ctrls=QDir(base+QStringLiteral("/nvme")).entryList(QDir::Dirs|QDir::NoDotAndDotDot);
        if(ctrls.isEmpty()) {
            // PCI function present, but no NVMe controller came up.
            QVariantMap d=common;
            d["name"]=QStringLiteral("PCIe-Gerät %1:%2 ohne NVMe-Funktion").arg(vid,did);
            d["romModeSuspect"]=true;
            d["sizeBytes"]=QString();
            d["devicePath"]=QString();
            d["protected"]=false;
            list.append(d);
            continue;
        }
        for(const auto &ctrl:ctrls) {
            const QString cbase=QStringLiteral("/sys/class/nvme/")+ctrl;
            const QString state=readSys(cbase+QStringLiteral("/state"));
            QStringList namespaces;
            for(const auto &b:QDir(QStringLiteral("/sys/block")).entryList(QDir::Dirs|QDir::NoDotAndDotDot|QDir::System))
                if(b.startsWith(ctrl+QLatin1Char('n'))) namespaces<<b;
            if(namespaces.isEmpty()) namespaces<<QString();
            for(const auto &ns:namespaces) {
                QVariantMap d=common;
                d["controllerPath"]=QStringLiteral("/dev/")+ctrl;
                d["nvmeState"]=state;
                d["name"]=readSys(cbase+QStringLiteral("/model"));
                d["serial"]=readSys(cbase+QStringLiteral("/serial"));
                d["firmwareSysfs"]=readSys(cbase+QStringLiteral("/firmware_rev"));
                d["namespace"]=ns;
                d["devicePath"]=ns.isEmpty() ? QString() : QStringLiteral("/dev/")+ns;
                d["sizeBytes"]=ns.isEmpty() ? QString() : QString::number(readSys(QStringLiteral("/sys/block/%1/size").arg(ns)).toULongLong()*512ULL);
                d["romModeSuspect"]=(state!=QStringLiteral("live"));
                const bool isProtected=!ns.isEmpty() && prot.contains(ns);
                d["protected"]=isProtected;
                if(isProtected) d["protectedReason"]=QStringLiteral("Boot-/Systemlaufwerk: %1").arg(reasons.join(QStringLiteral(", ")));
                d["mounted"]=ns.isEmpty() ? QStringList() : mountedPartitions(ns);
                list.append(d);
            }
        }
    }
#else
    Q_UNUSED(vendorIds)
#endif
    return list;
}

#ifndef Q_OS_WIN
// Raw admin command. Returns 0 on success, >0 NVMe status, <0 -errno.
inline int admin(int fd, quint8 opcode, quint32 nsid, void *data, quint32 len,
                 quint32 cdw10, quint32 cdw11=0, quint32 timeoutMs=30000, quint32 *result=nullptr) {
    struct nvme_admin_cmd cmd{};
    cmd.opcode=opcode;
    cmd.nsid=nsid;
    cmd.addr=quint64(reinterpret_cast<quintptr>(data));
    cmd.data_len=len;
    cmd.cdw10=cdw10;
    cmd.cdw11=cdw11;
    cmd.timeout_ms=timeoutMs;
    const int r=ioctl(fd,NVME_IOCTL_ADMIN_CMD,&cmd);
    if(result) *result=cmd.result;
    return r<0 ? -errno : r;
}
inline int getLog(int fd, quint8 lid, void *buf, quint32 len) {
    const quint32 numd=len/4-1;
    return admin(fd,0x02,0xffffffff,buf,len,(numd & 0xffff)<<16 | lid,numd>>16);
}
#endif

// Read-only inspection with the same field names as UsbBridge::inspect.
inline QVariantMap inspect(const QVariantMap &device) {
    QVariantMap r;
    r["usbConfirmed"]=false;
    r["present"]=false;
    r["nvmeIdentifyAvailable"]=false;
    r["smartAvailable"]=false;
    r["errorLogAvailable"]=false;
    r["recentNvmeErrors"]=QVariantList{};
#ifndef Q_OS_WIN
    const QString ctrl=device.value("controllerPath").toString();
    if(ctrl.isEmpty()) { r["present"]=QFileInfo::exists(QStringLiteral("/sys/bus/pci/devices/")+device.value("pciAddress").toString()); return r; }
    const int fd=::open(QFile::encodeName(ctrl).constData(),O_RDONLY|O_CLOEXEC);
    if(fd<0) { r["error"]=QStringLiteral("%1 nicht zugreifbar (%2)").arg(ctrl,QString::fromLocal8Bit(strerror(errno))); return r; }
    r["present"]=true;
    std::vector<unsigned char> id(4096,0);
    if(admin(fd,0x06,0,id.data(),4096,1)==0) {
        const auto str=[&](int off,int len){ return QString::fromLatin1(reinterpret_cast<const char*>(id.data()+off),len).trimmed(); };
        r["nvmeIdentifyAvailable"]=true;
        r["nvmePassthrough"]="Direkter NVMe-Zugriff (PCIe)";
        r["nvmeVendorId"]=QStringLiteral("%1").arg(unsigned(id[0])|(unsigned(id[1])<<8),4,16,QLatin1Char('0'));
        r["nvmeSubsystemVendorId"]=QStringLiteral("%1").arg(unsigned(id[2])|(unsigned(id[3])<<8),4,16,QLatin1Char('0'));
        r["nvmeControllerSerial"]=str(4,20);
        r["nvmeControllerModel"]=str(24,40);
        r["nvmeFirmware"]=str(64,8);
        r["oacs"]=unsigned(id[256])|(unsigned(id[257])<<8);
        r["frmw"]=unsigned(id[260]);
        r["fwug"]=unsigned(id[319]);
        r["firmwareDownloadSupported"]=((unsigned(id[256]) & 0x04)!=0);
        r["firmwareSlots"]=int((id[260]>>1)&0x7);
        r["firmwareSlot1ReadOnly"]=((id[260]&0x1)!=0);
        r["firmwareActivateWithoutReset"]=((id[260]&0x10)!=0);
    }
    unsigned char smart[512]{};
    if(admin(fd,0x02,0xffffffff,smart,512,((512/4-1)<<16)|0x02)==0) {
        const auto u128=[&](int off)->QString {
            for(int i=8;i<16;++i) if(smart[off+i]) return QStringLiteral("Größer als 64-Bit-Bereich");
            quint64 v=0; for(int i=7;i>=0;--i) v=(v<<8)|smart[off+i]; return QString::number(v);
        };
        const unsigned kelvin=unsigned(smart[1])|(unsigned(smart[2])<<8);
        r["smartAvailable"]=true;
        r["criticalWarning"]=QStringLiteral("0x%1").arg(unsigned(smart[0]),2,16,QLatin1Char('0'));
        if(kelvin>0 && kelvin<1000) r["temperatureCelsius"]=QString::number(int(kelvin)-273);
        r["availableSparePercent"]=QString::number(smart[3]);
        r["percentageUsed"]=QString::number(smart[5]);
        r["mediaErrors"]=u128(160);
        r["errorLogEntries"]=u128(176);
    }
    unsigned char err[16*64]{};
    if(getLog(fd,0x01,err,sizeof(err))==0) {
        QVariantList errors;
        for(int e=0;e<16;++e) {
            const unsigned char *p=err+e*64;
            quint64 count=0; for(int i=7;i>=0;--i) count=(count<<8)|p[i];
            if(!count) continue;
            const unsigned status=unsigned(p[12])|(unsigned(p[13])<<8);
            quint64 lba=0; for(int i=23;i>=16;--i) lba=(lba<<8)|p[i];
            errors.append(QVariantMap{{"errorCount",QString::number(count)},{"lba",QString::number(lba)},
                {"submissionQueueId",unsigned(p[8])|(unsigned(p[9])<<8)},{"commandId",unsigned(p[10])|(unsigned(p[11])<<8)},
                {"statusField",QStringLiteral("0x%1").arg(status,4,16,QLatin1Char('0'))},{"statusCode",(status>>1)&0xff},
                {"statusCodeType",(status>>9)&0x7},{"doNotRetry",(status&0x8000)!=0}});
        }
        r["recentNvmeErrors"]=errors;
        r["errorLogAvailable"]=true;
    }
    unsigned char fw[512]{};
    if(getLog(fd,0x03,fw,512)==0) {
        QVariantList slots;
        for(int s=0;s<7;++s) {
            const QString rev=QString::fromLatin1(reinterpret_cast<const char*>(fw+8+8*s),8).trimmed();
            slots.append(rev.isEmpty() ? QStringLiteral("leer") : rev);
        }
        r["firmwareSlotRevisions"]=slots;
        r["firmwareActiveSlot"]=int(fw[0]&0x7);
        r["firmwareNextSlot"]=int((fw[0]>>4)&0x7);
    }
    ::close(fd);
#else
    Q_UNUSED(device)
#endif
    return r;
}

#ifndef Q_OS_WIN
// Firmware Image Download (0x11) in granularity-sized pieces, then Firmware
// Commit (0x10). Only called by FirmwareManager after explicit approval.
// action: 1 = replace + activate at next reset, 3 = activate immediately.
inline QString flashFirmware(const QString &ctrl, const QByteArray &image, int slot, int action, quint32 granularity,
                             const std::function<void(int)> &progress) {
    if(image.isEmpty() || image.size()%4) return QStringLiteral("Firmwaredatei hat eine ungültige Größe.");
    const int fd=::open(QFile::encodeName(ctrl).constData(),O_RDWR|O_CLOEXEC);
    if(fd<0) return QStringLiteral("%1 nicht beschreibbar (%2).").arg(ctrl,QString::fromLocal8Bit(strerror(errno)));
    const quint32 piece=qBound<quint32>(4096,granularity,128*1024);
    void *buf=PlatformIO::allocAligned(piece,4096);
    QString error;
    for(qint64 off=0;off<image.size();off+=piece) {
        const quint32 len=quint32(qMin<qint64>(piece,image.size()-off));
        memcpy(buf,image.constData()+off,len);
        const int st=admin(fd,0x11,0,buf,len,len/4-1,quint32(off/4),60000);
        if(st!=0) { error=QStringLiteral("Firmware-Übertragung bei Byte %1 abgelehnt (Status %2).").arg(off).arg(st); break; }
        progress(int(100*(off+len)/image.size()));
    }
    PlatformIO::freeAligned(buf);
    if(error.isEmpty()) {
        const int st=admin(fd,0x10,0,nullptr,0,quint32((action&0x7)<<3)|quint32(slot&0x7),0,120000);
        // 0x10B / 0x110 / 0x111: commit succeeded, reset required for activation
        const int sc=st&0x7ff;
        if(st!=0 && sc!=0x10b && sc!=0x110 && sc!=0x111) error=QStringLiteral("Firmware-Commit abgelehnt (Status 0x%1).").arg(st,0,16);
        else if(st!=0 || action==1) {
            if(ioctl(fd,NVME_IOCTL_RESET)!=0) error=QStringLiteral("Firmware geschrieben, Controller-Reset fehlgeschlagen (%1). Neustart nötig.").arg(QString::fromLocal8Bit(strerror(errno)));
        }
    }
    ::close(fd);
    return error;
}
#endif

} // namespace LinuxNvme

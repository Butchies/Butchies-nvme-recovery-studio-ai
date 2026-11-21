#pragma once
// Writes the Linux Boot Edition image (.iso hybrid / .img) to a USB stick
// (Windows edition). Destructive for the stick only: target must be a USB
// disk, must not be the SSD being rescued, and the user must type the
// confirmation word. All volumes of the stick are locked and dismounted
// before raw writing; afterwards the written data is read back and compared.
#include <QString>
#include <QVariantMap>
#include <QFile>
#include <QFileInfo>
#include <QCryptographicHash>
#include <atomic>
#include <functional>
#include <vector>
#include "platform_io.h"
#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <winioctl.h>
#endif

namespace StickWriter {

constexpr const char *ConfirmWord="STICK";

inline QVariantMap write(const QString &image, const QString &diskPath, const QString &typed,
                         std::atomic_bool &cancel, const std::function<void(int,const QString&)> &progress) {
    QVariantMap r; r["ok"]=false;
    if(typed.trimmed().toUpper()!=QLatin1String(ConfirmWord)) { r["message"]=QStringLiteral("Bestätigung fehlt – nichts wurde geschrieben."); return r; }
    QFile in(image);
    if(!in.open(QIODevice::ReadOnly) || in.size()==0) { r["message"]=QStringLiteral("Abbilddatei nicht lesbar."); return r; }
#ifndef Q_OS_WIN
    Q_UNUSED(diskPath) Q_UNUSED(cancel) Q_UNUSED(progress)
    r["message"]=QStringLiteral("Unter Linux bitte linux-boot/make-stick.sh verwenden.");
    return r;
#else
    bool ok=false;
    const DWORD diskNumber=diskPath.mid(QStringLiteral("\\\\.\\PHYSICALDRIVE").size()).toUInt(&ok);
    if(!ok) { r["message"]=QStringLiteral("Ungültiger Ziel-Datenträger."); return r; }

    // Lock and dismount every volume located on the stick.
    std::vector<HANDLE> locked;
    wchar_t name[MAX_PATH]{};
    HANDLE find=FindFirstVolumeW(name,MAX_PATH);
    if(find!=INVALID_HANDLE_VALUE) {
        do {
            std::wstring vol(name);
            if(!vol.empty() && vol.back()==L'\\') vol.pop_back();
            HANDLE h=CreateFileW(vol.c_str(),GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,0,nullptr);
            if(h==INVALID_HANDLE_VALUE) continue;
            std::vector<unsigned char> buf(sizeof(VOLUME_DISK_EXTENTS)+31*sizeof(DISK_EXTENT));
            DWORD bytes=0; bool onStick=false;
            if(DeviceIoControl(h,IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,nullptr,0,buf.data(),DWORD(buf.size()),&bytes,nullptr)) {
                const auto *ex=reinterpret_cast<const VOLUME_DISK_EXTENTS*>(buf.data());
                for(DWORD i=0;i<ex->NumberOfDiskExtents && i<32;++i) if(ex->Extents[i].DiskNumber==diskNumber) onStick=true;
            }
            if(onStick && DeviceIoControl(h,FSCTL_LOCK_VOLUME,nullptr,0,nullptr,0,&bytes,nullptr)
                       && DeviceIoControl(h,FSCTL_DISMOUNT_VOLUME,nullptr,0,nullptr,0,&bytes,nullptr)) locked.push_back(h);
            else if(onStick) { CloseHandle(h); FindVolumeClose(find); for(auto l:locked) CloseHandle(l);
                r["message"]=QStringLiteral("Ein Laufwerk des Sticks ist in Benutzung und kann nicht getrennt werden. Explorer-Fenster schließen und erneut versuchen."); return r; }
            else CloseHandle(h);
        } while(FindNextVolumeW(find,name,MAX_PATH));
        FindVolumeClose(find);
    }
    struct Unlock { std::vector<HANDLE> &v; ~Unlock(){ DWORD b=0; for(auto h:v){ DeviceIoControl(h,FSCTL_UNLOCK_VOLUME,nullptr,0,nullptr,0,&b,nullptr); CloseHandle(h);} } } unlock{locked};

    const HANDLE disk=CreateFileW(PlatformIO::wide(diskPath),GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,
        OPEN_EXISTING,FILE_FLAG_NO_BUFFERING|FILE_FLAG_WRITE_THROUGH,nullptr);
    if(disk==INVALID_HANDLE_VALUE) { r["message"]=QStringLiteral("Stick kann nicht beschrieben werden (%1).").arg(GetLastError()); return r; }
    struct Close { HANDLE h; ~Close(){ DWORD b=0; DeviceIoControl(h,IOCTL_DISK_UPDATE_PROPERTIES,nullptr,0,nullptr,0,&b,nullptr); CloseHandle(h);} } closer{disk};
    DWORD bytes=0;
    GET_LENGTH_INFORMATION len{};
    DISK_GEOMETRY_EX geo{};
    if(!DeviceIoControl(disk,IOCTL_DISK_GET_LENGTH_INFO,nullptr,0,&len,sizeof(len),&bytes,nullptr) ||
       !DeviceIoControl(disk,IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,nullptr,0,&geo,sizeof(geo),&bytes,nullptr)) {
        r["message"]=QStringLiteral("Größe des Sticks nicht ermittelbar."); return r;
    }
    if(quint64(in.size())>quint64(len.Length.QuadPart)) { r["message"]=QStringLiteral("Der Stick ist zu klein für das Abbild."); return r; }
    const quint32 sector=geo.Geometry.BytesPerSector;
    const quint32 chunk=4*1024*1024;
    void *buf=PlatformIO::allocAligned(chunk,sector);
    void *check=PlatformIO::allocAligned(chunk,sector);
    QCryptographicHash written(QCryptographicHash::Sha256), readBack(QCryptographicHash::Sha256);
    const qint64 total=in.size();
    QString error;
    for(qint64 off=0;off<total && error.isEmpty();off+=chunk) {
        if(cancel.load()) { error=QStringLiteral("Abgebrochen – der Stick ist unvollständig und muss neu beschrieben werden."); break; }
        memset(buf,0,chunk);
        const qint64 n=in.read(static_cast<char*>(buf),chunk);
        if(n<=0) { error=QStringLiteral("Lesefehler im Abbild."); break; }
        const quint32 aligned=quint32((n+sector-1)/sector*sector);
        written.addData(QByteArrayView(static_cast<const char*>(buf),n));
        LARGE_INTEGER li; li.QuadPart=off;
        DWORD done=0;
        if(!SetFilePointerEx(disk,li,nullptr,FILE_BEGIN) || !WriteFile(disk,buf,aligned,&done,nullptr) || done!=aligned)
            error=QStringLiteral("Schreibfehler bei Byte %1 (%2).").arg(off).arg(GetLastError());
        progress(int(80*(off+n)/total),QStringLiteral("Schreibe Abbild …"));
    }
    for(qint64 off=0;off<total && error.isEmpty();off+=chunk) {
        const quint32 n=quint32(qMin<qint64>(chunk,total-off));
        const quint32 aligned=(n+sector-1)/sector*sector;
        LARGE_INTEGER li; li.QuadPart=off;
        DWORD done=0;
        if(!SetFilePointerEx(disk,li,nullptr,FILE_BEGIN) || !ReadFile(disk,check,aligned,&done,nullptr) || done!=aligned) { error=QStringLiteral("Prüflesen fehlgeschlagen bei Byte %1.").arg(off); break; }
        readBack.addData(QByteArrayView(static_cast<const char*>(check),n));
        progress(80+int(20*(off+n)/total),QStringLiteral("Prüfe geschriebene Daten …"));
    }
    PlatformIO::freeAligned(buf);
    PlatformIO::freeAligned(check);
    if(!error.isEmpty()) { r["message"]=error; return r; }
    const bool same=written.result()==readBack.result();
    r["ok"]=same;
    r["sha256"]=QString::fromLatin1(written.result().toHex());
    r["message"]=same ? QStringLiteral("Linux-Boot-Stick erstellt und geprüft. Beim Start des Rechners vom Stick booten (UEFI). Der dauerhafte Speicherbereich wird beim ersten Start automatisch angelegt.")
                      : QStringLiteral("Prüfung fehlgeschlagen: Gelesene Daten weichen ab. Stick defekt oder getrennt?");
    return r;
#endif
}

} // namespace StickWriter

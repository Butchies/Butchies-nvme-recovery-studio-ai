#pragma once
// Platform layer for the rescue engine: read-only source device with
// per-read timeout, image file, aligned buffers and the check that the
// destination does not reside on the source disk.
//   Windows: \\.\PHYSICALDRIVEn, overlapped I/O + CancelIoEx
//   Linux:   /dev/nvmeXnY, /dev/sdX …, O_DIRECT + kernel AIO with timeout
#include <QString>
#include <QStringList>
#include <QDir>
#include <QFileInfo>
#include <QSet>
#include <QThread>
#include <QFile>
#include <cstddef>
#include <cstring>
#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <winioctl.h>
#include <vector>
#include <string>
#else
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/syscall.h>
#include <sys/sysmacros.h>
#include <linux/fs.h>
#include <linux/aio_abi.h>
#include <time.h>
#endif

namespace PlatformIO {

enum class ReadStatus { Ok, Error, Timeout, Hung, DeviceLost, Rejected };
struct ReadResult { ReadStatus status=ReadStatus::Ok; long code=0; };

inline QString describe(const ReadResult &r) {
    switch(r.status) {
    case ReadStatus::Ok: return QStringLiteral("ok");
    case ReadStatus::Timeout: return QStringLiteral("Zeitüberschreitung");
    case ReadStatus::Hung: return QStringLiteral("Gerät reagiert nicht mehr");
    case ReadStatus::DeviceLost: return QStringLiteral("Verbindung verloren (%1)").arg(r.code);
    case ReadStatus::Rejected: return QStringLiteral("Befehl abgelehnt (%1)").arg(r.code);
    default: return QStringLiteral("Lesefehler (%1)").arg(r.code);
    }
}

inline void *allocAligned(size_t size, size_t align) {
#ifdef Q_OS_WIN
    Q_UNUSED(align)
    return VirtualAlloc(nullptr,SIZE_T(size),MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
#else
    void *p=nullptr;
    return posix_memalign(&p,qMax<size_t>(align,4096),size)==0 ? p : nullptr;
#endif
}
inline void freeAligned(void *p) {
#ifdef Q_OS_WIN
    if(p) VirtualFree(p,0,MEM_RELEASE);
#else
    free(p);
#endif
}

#ifdef Q_OS_WIN
inline LPCWSTR wide(const QString &s) { return reinterpret_cast<LPCWSTR>(s.utf16()); }
#endif

// ---------------------------------------------------------------------------
class SourceDevice {
public:
    ~SourceDevice() { close(); }
    bool open(const QString &path, QString &error) {
        close();
        m_path=path;
#ifdef Q_OS_WIN
        m_handle=CreateFileW(wide(path),GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,
            OPEN_EXISTING,FILE_FLAG_NO_BUFFERING|FILE_FLAG_OVERLAPPED,nullptr);
        if(m_handle==INVALID_HANDLE_VALUE) {
            const DWORD e=GetLastError();
            error=e==ERROR_ACCESS_DENIED ? QStringLiteral("Lesezugriff auf die SSD verweigert. Programm als Administrator starten.")
                : QStringLiteral("Quelllaufwerk kann nicht lesend geöffnet werden (Windows-Fehler %1).").arg(e);
            return false;
        }
        m_event=CreateEventW(nullptr,TRUE,FALSE,nullptr);
        DWORD bytes=0;
        DISK_GEOMETRY_EX geo{};
        if(!m_event || !DeviceIoControl(m_handle,IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,nullptr,0,&geo,sizeof(geo),&bytes,nullptr)) {
            error=QStringLiteral("Sektorgröße der SSD nicht ermittelbar (%1).").arg(GetLastError()); close(); return false;
        }
        m_sector=geo.Geometry.BytesPerSector;
        GET_LENGTH_INFORMATION len{};
        if(!DeviceIoControl(m_handle,IOCTL_DISK_GET_LENGTH_INFO,nullptr,0,&len,sizeof(len),&bytes,nullptr)) {
            error=QStringLiteral("Kapazität der SSD nicht ermittelbar (%1).").arg(GetLastError()); close(); return false;
        }
        m_size=quint64(len.Length.QuadPart);
#else
        m_fd=::open(QFile::encodeName(path).constData(),O_RDONLY|O_DIRECT|O_CLOEXEC);
        if(m_fd<0) {
            error=errno==EACCES ? QStringLiteral("Lesezugriff auf die SSD verweigert (Root-Rechte nötig).")
                : QStringLiteral("Quelllaufwerk kann nicht lesend geöffnet werden (%1).").arg(QString::fromLocal8Bit(strerror(errno)));
            return false;
        }
        int sector=0; unsigned long long size=0;
        if(ioctl(m_fd,BLKSSZGET,&sector)!=0 || ioctl(m_fd,BLKGETSIZE64,&size)!=0) {
            error=QStringLiteral("Sektorgröße/Kapazität nicht ermittelbar."); close(); return false;
        }
        m_sector=quint32(sector); m_size=size;
        if(syscall(SYS_io_setup,1,&m_ctx)!=0) { error=QStringLiteral("Kernel-AIO nicht verfügbar."); close(); return false; }
#endif
        if(m_sector<512 || m_sector>65536 || (m_sector&(m_sector-1))) { error=QStringLiteral("Ungültige Sektorgröße %1.").arg(m_sector); close(); return false; }
        m_size-=m_size%m_sector;
        return true;
    }
    bool reopen() { QString e; const QString p=m_path; return open(p,e); }
    void close() {
        if(m_hung) return; // pending I/O may still use handle and buffer
#ifdef Q_OS_WIN
        if(m_handle!=INVALID_HANDLE_VALUE) CloseHandle(m_handle);
        if(m_event) CloseHandle(m_event);
        m_handle=INVALID_HANDLE_VALUE; m_event=nullptr;
#else
        if(m_ctx) { syscall(SYS_io_destroy,m_ctx); m_ctx=0; }
        if(m_fd>=0) ::close(m_fd);
        m_fd=-1;
#endif
    }
    quint32 sectorSize() const { return m_sector; }
    quint64 size() const { return m_size; }
    bool hung() const { return m_hung; }

    ReadResult read(quint64 offset, void *buffer, quint32 length, quint32 timeoutMs) {
        if(m_hung) return {ReadStatus::Hung,0};
#ifdef Q_OS_WIN
        OVERLAPPED ov{};
        ov.Offset=DWORD(offset & 0xffffffffULL);
        ov.OffsetHigh=DWORD(offset>>32);
        ov.hEvent=m_event;
        ResetEvent(m_event);
        DWORD got=0;
        if(!ReadFile(m_handle,buffer,length,nullptr,&ov)) {
            const DWORD e=GetLastError();
            if(e!=ERROR_IO_PENDING) return classify(e);
            if(WaitForSingleObject(m_event,timeoutMs)==WAIT_TIMEOUT) {
                CancelIoEx(m_handle,&ov);
                if(!GetOverlappedResultEx(m_handle,&ov,&got,10000,FALSE) && GetLastError()==WAIT_TIMEOUT) {
                    m_hung=true; return {ReadStatus::Hung,0};
                }
                return {ReadStatus::Timeout,0};
            }
        }
        if(!GetOverlappedResult(m_handle,&ov,&got,TRUE)) return classify(GetLastError());
        return got==length ? ReadResult{} : ReadResult{ReadStatus::Error,long(ERROR_READ_FAULT)};
#else
        struct iocb cb{};
        cb.aio_lio_opcode=IOCB_CMD_PREAD;
        cb.aio_fildes=quint32(m_fd);
        cb.aio_buf=quint64(reinterpret_cast<quintptr>(buffer));
        cb.aio_nbytes=length;
        cb.aio_offset=qint64(offset);
        struct iocb *list[1]={&cb};
        if(syscall(SYS_io_submit,m_ctx,1,list)!=1) return classify(errno);
        struct io_event ev{};
        struct timespec ts{long(timeoutMs/1000),long(timeoutMs%1000)*1000000L};
        const long n=syscall(SYS_io_getevents,m_ctx,1,1,&ev,&ts);
        if(n!=1) {
            struct io_event cancelled{};
            if(syscall(SYS_io_cancel,m_ctx,&cb,&cancelled)==0) return {ReadStatus::Timeout,0};
            struct timespec grace{10,0};
            if(syscall(SYS_io_getevents,m_ctx,1,1,&ev,&grace)==1) return {ReadStatus::Timeout,0};
            m_hung=true; return {ReadStatus::Hung,0};
        }
        if(ev.res<0) return classify(int(-ev.res));
        return quint64(ev.res)==length ? ReadResult{} : ReadResult{ReadStatus::Error,EIO};
#endif
    }
private:
#ifdef Q_OS_WIN
    static ReadResult classify(DWORD e) {
        if(e==ERROR_DEVICE_NOT_CONNECTED || e==ERROR_NO_SUCH_DEVICE || e==ERROR_DEV_NOT_EXIST || e==ERROR_FILE_NOT_FOUND ||
           e==ERROR_INVALID_HANDLE || e==ERROR_NOT_READY || e==ERROR_DEVICE_REMOVED) return {ReadStatus::DeviceLost,long(e)};
        if(e==ERROR_INVALID_PARAMETER) return {ReadStatus::Rejected,long(e)};
        return {ReadStatus::Error,long(e)};
    }
    HANDLE m_handle=INVALID_HANDLE_VALUE;
    HANDLE m_event=nullptr;
#else
    static ReadResult classify(int e) {
        if(e==ENODEV || e==ENXIO || e==ENOMEDIUM || e==EBADF) return {ReadStatus::DeviceLost,e};
        if(e==EINVAL) return {ReadStatus::Rejected,e};
        return {ReadStatus::Error,e};
    }
    int m_fd=-1;
    aio_context_t m_ctx=0;
#endif
    QString m_path;
    quint32 m_sector=512;
    quint64 m_size=0;
    bool m_hung=false;
};

// ---------------------------------------------------------------------------
class ImageFile {
public:
    ~ImageFile() { close(); }
    bool open(const QString &path, bool create, QString &error) {
#ifdef Q_OS_WIN
        m_h=CreateFileW(wide(QDir::toNativeSeparators(path)),GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,nullptr,
            create ? CREATE_NEW : OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
        if(m_h==INVALID_HANDLE_VALUE) { error=QStringLiteral("Abbilddatei kann nicht geöffnet werden (%1).").arg(GetLastError()); return false; }
        if(create) { DWORD b=0; DeviceIoControl(m_h,FSCTL_SET_SPARSE,nullptr,0,nullptr,0,&b,nullptr); }
#else
        m_fd=::open(QFile::encodeName(path).constData(),O_RDWR|O_CLOEXEC|(create ? (O_CREAT|O_EXCL) : 0),0644);
        if(m_fd<0) { error=QStringLiteral("Abbilddatei kann nicht geöffnet werden (%1).").arg(QString::fromLocal8Bit(strerror(errno))); return false; }
#endif
        return true;
    }
    bool resize(quint64 size) {
#ifdef Q_OS_WIN
        FILE_END_OF_FILE_INFO eof{}; eof.EndOfFile.QuadPart=LONGLONG(size);
        return SetFileInformationByHandle(m_h,FileEndOfFileInfo,&eof,sizeof(eof));
#else
        return ftruncate(m_fd,off_t(size))==0;
#endif
    }
    bool write(quint64 offset, const void *data, quint32 length) {
#ifdef Q_OS_WIN
        LARGE_INTEGER li; li.QuadPart=LONGLONG(offset);
        DWORD written=0;
        return SetFilePointerEx(m_h,li,nullptr,FILE_BEGIN) && WriteFile(m_h,data,length,&written,nullptr) && written==length;
#else
        const char *p=static_cast<const char*>(data);
        quint32 done=0;
        while(done<length) {
            const ssize_t n=pwrite(m_fd,p+done,length-done,off_t(offset+done));
            if(n<0 && errno==EINTR) continue;
            if(n<=0) return false;
            done+=quint32(n);
        }
        return true;
#endif
    }
    void flush() {
#ifdef Q_OS_WIN
        FlushFileBuffers(m_h);
#else
        fdatasync(m_fd);
#endif
    }
    quint64 size() const {
#ifdef Q_OS_WIN
        LARGE_INTEGER s{}; return GetFileSizeEx(m_h,&s) ? quint64(s.QuadPart) : 0;
#else
        struct stat st{}; return fstat(m_fd,&st)==0 ? quint64(st.st_size) : 0;
#endif
    }
    void close() {
#ifdef Q_OS_WIN
        if(m_h!=INVALID_HANDLE_VALUE) CloseHandle(m_h);
        m_h=INVALID_HANDLE_VALUE;
#else
        if(m_fd>=0) ::close(m_fd);
        m_fd=-1;
#endif
    }
    QString lastError() const {
#ifdef Q_OS_WIN
        return QString::number(GetLastError());
#else
        return QString::fromLocal8Bit(strerror(errno));
#endif
    }
private:
#ifdef Q_OS_WIN
    HANDLE m_h=INVALID_HANDLE_VALUE;
#else
    int m_fd=-1;
#endif
};

#ifndef Q_OS_WIN
// Physical disks (e.g. "nvme0n1", "sda") behind a block device number,
// resolving partitions and device-mapper/MD stacks.
inline QSet<QString> diskNamesForDev(dev_t dev) {
    QSet<QString> out;
    QString sys=QFileInfo(QStringLiteral("/sys/dev/block/%1:%2").arg(major(dev)).arg(minor(dev))).canonicalFilePath();
    if(sys.isEmpty()) return out;
    if(QFileInfo::exists(sys+QStringLiteral("/partition"))) sys=QFileInfo(sys).path();
    const QString name=QFileInfo(sys).fileName();
    const QStringList slaves=QDir(sys+QStringLiteral("/slaves")).entryList(QDir::Dirs|QDir::NoDotAndDotDot);
    if(slaves.isEmpty()) { out.insert(name); return out; }
    for(const auto &s:slaves) {
        struct stat st{};
        if(::stat(QFile::encodeName(QStringLiteral("/dev/")+s).constData(),&st)==0) out.unite(diskNamesForDev(st.st_rdev));
        else out.insert(s);
    }
    return out;
}
#endif

// True if the destination folder is on a different physical disk than the
// source device. Unknown situations are treated as unsafe.
inline bool destinationOnSeparateDisk(const QString &dir, const QString &sourcePath, QString &error) {
#ifdef Q_OS_WIN
    bool ok=false;
    const quint32 sourceDisk=sourcePath.mid(QStringLiteral("\\\\.\\PHYSICALDRIVE").size()).toUInt(&ok);
    if(!ok) { error=QStringLiteral("Ungültige Laufwerksnummer."); return false; }
    const QString native=QDir::toNativeSeparators(QFileInfo(dir).absoluteFilePath());
    wchar_t volumePath[MAX_PATH+1]{};
    if(!GetVolumePathNameW(wide(native),volumePath,MAX_PATH)) { error=QStringLiteral("Ziellaufwerk nicht bestimmbar."); return false; }
    if(GetDriveTypeW(volumePath)==DRIVE_REMOTE) return true;
    wchar_t volumeName[MAX_PATH+1]{};
    if(!GetVolumeNameForVolumeMountPointW(volumePath,volumeName,MAX_PATH)) { error=QStringLiteral("Zielvolume nicht eindeutig bestimmbar; Rettung gesperrt."); return false; }
    std::wstring device(volumeName);
    if(!device.empty() && device.back()==L'\\') device.pop_back();
    const HANDLE volume=CreateFileW(device.c_str(),0,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,0,nullptr);
    if(volume==INVALID_HANDLE_VALUE) { error=QStringLiteral("Zielvolume kann nicht geprüft werden (%1); Rettung gesperrt.").arg(GetLastError()); return false; }
    std::vector<unsigned char> buffer(sizeof(VOLUME_DISK_EXTENTS)+31*sizeof(DISK_EXTENT));
    DWORD bytes=0;
    const BOOL got=DeviceIoControl(volume,IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,nullptr,0,buffer.data(),DWORD(buffer.size()),&bytes,nullptr);
    CloseHandle(volume);
    if(!got) { error=QStringLiteral("Datenträgerzuordnung des Ziels nicht ermittelbar; Rettung gesperrt."); return false; }
    const auto *extents=reinterpret_cast<const VOLUME_DISK_EXTENTS*>(buffer.data());
    for(DWORD i=0;i<extents->NumberOfDiskExtents && i<32;++i)
        if(extents->Extents[i].DiskNumber==sourceDisk) { error=QStringLiteral("Der Zielordner liegt auf der zu rettenden SSD. Bitte ein anderes Laufwerk wählen."); return false; }
    return true;
#else
    struct statfs sfs{};
    if(statfs(QFile::encodeName(dir).constData(),&sfs)!=0) { error=QStringLiteral("Ziellaufwerk nicht bestimmbar."); return false; }
    const auto type=static_cast<unsigned long>(sfs.f_type);
    if(type==0x01021994UL || type==0x794c7630UL || type==0x858458f6UL) {
        error=QStringLiteral("Der Zielordner liegt im Arbeitsspeicher des Live-Systems (tmpfs/overlay). Bitte ein echtes Laufwerk oder den Speicherbereich des Sticks wählen.");
        return false;
    }
    if(type==0x6969UL || type==0xFF534D42UL || type==0xFE534D42UL) return true; // NFS / SMB
    struct stat dst{}, src{};
    if(::stat(QFile::encodeName(dir).constData(),&dst)!=0 || ::stat(QFile::encodeName(sourcePath).constData(),&src)!=0 || !S_ISBLK(src.st_mode)) {
        error=QStringLiteral("Quelle oder Ziel nicht bestimmbar; Rettung gesperrt."); return false;
    }
    const auto destDisks=diskNamesForDev(dst.st_dev);
    const auto srcDisks=diskNamesForDev(src.st_rdev);
    if(destDisks.isEmpty() || srcDisks.isEmpty()) { error=QStringLiteral("Datenträgerzuordnung nicht ermittelbar; Rettung gesperrt."); return false; }
    if(destDisks.intersects(srcDisks)) { error=QStringLiteral("Der Zielordner liegt auf der zu rettenden SSD. Bitte ein anderes Laufwerk wählen."); return false; }
    return true;
#endif
}

} // namespace PlatformIO

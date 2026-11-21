#pragma once
#include <QString>
#include <QVariantMap>
#ifdef Q_OS_WIN
#include <windows.h>
#include <winioctl.h>
#include <vector>
#include <cstddef>
#include <cstring>
#endif

namespace UsbBridge {
inline QVariantMap inspect(const QString &path) {
    QVariantMap report;
    report["storageBus"]="Nicht ermittelt";
    report["deviceFirmware"]="Nicht ermittelt";
    report["deviceVendor"]="Nicht ermittelt";
    report["deviceProduct"]="Nicht ermittelt";
    report["nvmePassthrough"]="Nicht nachgewiesen";
    report["usbConfirmed"]=false;
    report["nvmeIdentifyAvailable"]=false;
    report["nvmeControllerModel"]="Nicht ermittelt";
    report["nvmeControllerSerial"]="Nicht ermittelt";
    report["nvmeFirmware"]="Nicht ermittelt";
    report["error"]="";
#ifdef Q_OS_WIN
    if (!path.startsWith(QStringLiteral("\\\\.\\PHYSICALDRIVE"),Qt::CaseInsensitive)) {
        report["error"]="Kein physischer Windows-Laufwerkpfad.";return report;
    }
    bool valid=false;
    path.mid(QStringLiteral("\\\\.\\PHYSICALDRIVE").size()).toUInt(&valid);
    if (!valid) {report["error"]="Ungültige Laufwerksnummer.";return report;}
    const HANDLE handle=CreateFileW(reinterpret_cast<LPCWSTR>(path.utf16()),
        0,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,0,nullptr);
    if(handle==INVALID_HANDLE_VALUE) {
        report["error"]=QStringLiteral("Lesender Geräte-Metadatenzugriff nicht möglich (%1).").arg(GetLastError());
        return report;
    }
    STORAGE_PROPERTY_QUERY query{};
    query.PropertyId=StorageDeviceProperty;
    query.QueryType=PropertyStandardQuery;
    std::vector<unsigned char> buffer(4096);
    DWORD received=0;
    if(DeviceIoControl(handle,IOCTL_STORAGE_QUERY_PROPERTY,&query,sizeof(query),
            buffer.data(),static_cast<DWORD>(buffer.size()),&received,nullptr)
       && received>=sizeof(STORAGE_DEVICE_DESCRIPTOR)) {
        const auto *desc=reinterpret_cast<const STORAGE_DEVICE_DESCRIPTOR*>(buffer.data());
        const bool usb=desc->BusType==BusTypeUsb;
        report["usbConfirmed"]=usb;
        report["storageBus"]=usb ? "USB" : QStringLiteral("Andere Busart (%1)").arg(int(desc->BusType));
        const auto extract=[&](DWORD offset)->QString {
            if(offset==0 || offset>=received) return QStringLiteral("Nicht ermittelt");
            const auto *start=reinterpret_cast<const char*>(buffer.data()+offset);
            const auto remaining=received-offset;
            const auto *end=static_cast<const char*>(memchr(start,0,remaining));
            if(!end) return QStringLiteral("Nicht ermittelt");
            return QString::fromLatin1(start,qsizetype(end-start)).trimmed();
        };
        report["deviceVendor"]=extract(desc->VendorIdOffset);
        report["deviceProduct"]=extract(desc->ProductIdOffset);
        report["deviceFirmware"]=extract(desc->ProductRevisionOffset);
        if(!usb) report["error"]="Windows meldet keinen USB-Speicherbus; Gerät gesperrt.";
    } else {
        report["error"]=QStringLiteral("STORAGE_DEVICE_DESCRIPTOR nicht verfügbar (%1).").arg(GetLastError());
    }
    if(report.value("usbConfirmed").toBool()) {
        // Standard Windows storage protocol query: NVMe Identify Controller,
        // data-in only. Unsupported USB bridges fail without attempting writes.
        struct ProtocolQuery {
            STORAGE_PROPERTY_QUERY property;
            STORAGE_PROTOCOL_SPECIFIC_DATA protocol;
            unsigned char identify[4096];
        };
        ProtocolQuery request{};
        request.property.PropertyId=StorageDeviceProtocolSpecificProperty;
        request.property.QueryType=PropertyStandardQuery;
        request.protocol.ProtocolType=ProtocolTypeNvme;
        request.protocol.DataType=NVMeDataTypeIdentify;
        request.protocol.ProtocolDataRequestValue=1; // CNS=1: Identify Controller
        request.protocol.ProtocolDataOffset=sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA);
        request.protocol.ProtocolDataLength=4096;
        DWORD bytes=0;
        if(DeviceIoControl(handle,IOCTL_STORAGE_QUERY_PROPERTY,
                &request,static_cast<DWORD>(offsetof(ProtocolQuery,identify)),
                &request,sizeof(request),&bytes,nullptr)) {
            const auto *protocol=reinterpret_cast<const STORAGE_PROTOCOL_SPECIFIC_DATA*>(
                reinterpret_cast<const unsigned char*>(&request)+sizeof(STORAGE_PROPERTY_QUERY));
            const auto base=reinterpret_cast<const unsigned char*>(protocol);
            const auto begin=reinterpret_cast<const unsigned char*>(&request);
            const auto end=begin+qMin<size_t>(bytes,sizeof(request));
            const auto data=base+protocol->ProtocolDataOffset;
            if(protocol->ProtocolType==ProtocolTypeNvme &&
               protocol->DataType==NVMeDataTypeIdentify &&
               protocol->ProtocolDataLength>=4096 &&
               data>=begin && data<=end && size_t(end-data)>=4096) {
                const auto vendor=unsigned(data[0]) | (unsigned(data[1])<<8);
                const auto model=QString::fromLatin1(reinterpret_cast<const char*>(data+24),40).trimmed();
                const auto serial=QString::fromLatin1(reinterpret_cast<const char*>(data+4),20).trimmed();
                const auto firmware=QString::fromLatin1(reinterpret_cast<const char*>(data+64),8).trimmed();
                if(vendor!=0 && vendor!=0xffff && !model.isEmpty() && !serial.isEmpty()) {
                    report["nvmeIdentifyAvailable"]=true;
                    report["nvmePassthrough"]="NVMe Identify Controller lesend verfügbar";
                    report["nvmeControllerModel"]=model;
                    report["nvmeControllerSerial"]=serial;
                    report["nvmeFirmware"]=firmware;
                }
            }
        }
    }
    CloseHandle(handle);
#else
    Q_UNUSED(path)
    report["error"]="Nur Windows.";
#endif
    return report;
}
}

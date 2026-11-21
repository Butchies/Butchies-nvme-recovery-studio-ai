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
    report["smartAvailable"]=false;
    report["temperatureCelsius"]="Nicht ermittelt";
    report["criticalWarning"]="Nicht ermittelt";
    report["availableSparePercent"]="Nicht ermittelt";
    report["percentageUsed"]="Nicht ermittelt";
    report["mediaErrors"]="Nicht ermittelt";
    report["errorLogEntries"]="Nicht ermittelt";
    report["errorLogAvailable"]=false;
    report["recentNvmeErrors"]=QVariantList{};
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
        constexpr size_t header=offsetof(STORAGE_PROPERTY_QUERY,AdditionalParameters);
        std::vector<unsigned char> protocolBuffer(
            header+sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA)+4096,0);
        auto *property=reinterpret_cast<STORAGE_PROPERTY_QUERY*>(protocolBuffer.data());
        property->PropertyId=StorageDeviceProtocolSpecificProperty;
        property->QueryType=PropertyStandardQuery;
        auto *specific=reinterpret_cast<STORAGE_PROTOCOL_SPECIFIC_DATA*>(
            protocolBuffer.data()+header);
        specific->ProtocolType=ProtocolTypeNvme;
        specific->DataType=NVMeDataTypeIdentify;
        specific->ProtocolDataRequestValue=1; // CNS=1: Identify Controller
        specific->ProtocolDataOffset=sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA);
        specific->ProtocolDataLength=4096;
        DWORD bytes=0;
        if(DeviceIoControl(handle,IOCTL_STORAGE_QUERY_PROPERTY,
                protocolBuffer.data(),static_cast<DWORD>(header+sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA)),
                protocolBuffer.data(),static_cast<DWORD>(protocolBuffer.size()),&bytes,nullptr)) {
            const auto *protocol=reinterpret_cast<const STORAGE_PROTOCOL_SPECIFIC_DATA*>(
                protocolBuffer.data()+header);
            const auto base=reinterpret_cast<const unsigned char*>(protocol);
            const auto begin=protocolBuffer.data();
            const auto end=begin+qMin<size_t>(bytes,protocolBuffer.size());
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
    if(report.value("nvmeIdentifyAvailable").toBool()) {
        // NVMe SMART/Health log page 02h, 512 bytes, strictly data-in.
        constexpr size_t header=offsetof(STORAGE_PROPERTY_QUERY,AdditionalParameters);
        std::vector<unsigned char> healthBuffer(
            header+sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA)+512,0);
        auto *property=reinterpret_cast<STORAGE_PROPERTY_QUERY*>(healthBuffer.data());
        property->PropertyId=StorageDeviceProtocolSpecificProperty;
        property->QueryType=PropertyStandardQuery;
        auto *specific=reinterpret_cast<STORAGE_PROTOCOL_SPECIFIC_DATA*>(
            healthBuffer.data()+header);
        specific->ProtocolType=ProtocolTypeNvme;
        specific->DataType=NVMeDataTypeLogPage;
        specific->ProtocolDataRequestValue=0x02; // SMART / Health Information
        specific->ProtocolDataRequestSubValue=0xffffffff; // Controller-wide namespace
        specific->ProtocolDataOffset=sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA);
        specific->ProtocolDataLength=512;
        DWORD bytes=0;
        if(DeviceIoControl(handle,IOCTL_STORAGE_QUERY_PROPERTY,
                healthBuffer.data(),static_cast<DWORD>(header+sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA)),
                healthBuffer.data(),static_cast<DWORD>(healthBuffer.size()),&bytes,nullptr)) {
            const auto *protocol=reinterpret_cast<const STORAGE_PROTOCOL_SPECIFIC_DATA*>(
                healthBuffer.data()+header);
            const auto *begin=healthBuffer.data();
            const size_t available=qMin<size_t>(bytes,healthBuffer.size());
            const size_t baseOffset=header+protocol->ProtocolDataOffset;
            if(protocol->ProtocolType==ProtocolTypeNvme &&
               protocol->DataType==NVMeDataTypeLogPage &&
               protocol->ProtocolDataLength>=512 &&
               baseOffset<=available && available-baseOffset>=512) {
                const auto *data=begin+baseOffset;
                const auto kelvin=unsigned(data[1])|(unsigned(data[2])<<8);
                const auto uint128Text=[&](size_t offset)->QString {
                    // Decimal conversion with overflow protection: report lower 64 bits
                    // only when the upper 64 bits are all zero.
                    for(size_t i=8;i<16;++i) if(data[offset+i]!=0)
                        return QStringLiteral("Größer als 64-Bit-Bereich");
                    quint64 value=0;
                    for(int i=7;i>=0;--i) value=(value<<8)|data[offset+i];
                    return QString::number(value);
                };
                report["smartAvailable"]=true;
                report["criticalWarning"]=QStringLiteral("0x%1").arg(unsigned(data[0]),2,16,QLatin1Char('0'));
                if(kelvin>0 && kelvin<1000)
                    report["temperatureCelsius"]=QString::number(int(kelvin)-273);
                report["availableSparePercent"]=QString::number(unsigned(data[3]));
                report["percentageUsed"]=QString::number(unsigned(data[5]));
                report["mediaErrors"]=uint128Text(160);
                report["errorLogEntries"]=uint128Text(176);
            }
        }
    }
    if(report.value("nvmeIdentifyAvailable").toBool()) {
        // Read the first four entries of NVMe Error Information Log (LID 01h).
        // No reset, recovery command or device write is issued.
        constexpr size_t header=offsetof(STORAGE_PROPERTY_QUERY,AdditionalParameters);
        constexpr size_t logLength=4*64;
        std::vector<unsigned char> logBuffer(
            header+sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA)+logLength,0);
        auto *property=reinterpret_cast<STORAGE_PROPERTY_QUERY*>(logBuffer.data());
        property->PropertyId=StorageDeviceProtocolSpecificProperty;
        property->QueryType=PropertyStandardQuery;
        auto *specific=reinterpret_cast<STORAGE_PROTOCOL_SPECIFIC_DATA*>(
            logBuffer.data()+header);
        specific->ProtocolType=ProtocolTypeNvme;
        specific->DataType=NVMeDataTypeLogPage;
        specific->ProtocolDataRequestValue=0x01;
        specific->ProtocolDataRequestSubValue=0xffffffff;
        specific->ProtocolDataOffset=sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA);
        specific->ProtocolDataLength=logLength;
        DWORD bytes=0;
        if(DeviceIoControl(handle,IOCTL_STORAGE_QUERY_PROPERTY,
                logBuffer.data(),static_cast<DWORD>(header+sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA)),
                logBuffer.data(),static_cast<DWORD>(logBuffer.size()),&bytes,nullptr)) {
            const auto *protocol=reinterpret_cast<const STORAGE_PROTOCOL_SPECIFIC_DATA*>(
                logBuffer.data()+header);
            const size_t available=qMin<size_t>(bytes,logBuffer.size());
            const size_t baseOffset=header+protocol->ProtocolDataOffset;
            if(protocol->ProtocolType==ProtocolTypeNvme &&
               protocol->DataType==NVMeDataTypeLogPage &&
               protocol->ProtocolDataLength>=logLength &&
               baseOffset<=available && available-baseOffset>=logLength) {
                const auto *data=logBuffer.data()+baseOffset;
                QVariantList errors;
                for(size_t entry=0;entry<4;++entry) {
                    const auto *p=data+entry*64;
                    quint64 count=0;
                    for(int i=7;i>=0;--i) count=(count<<8)|p[i];
                    if(count==0) continue;
                    const auto status=unsigned(p[12])|(unsigned(p[13])<<8);
                    const auto sqid=unsigned(p[8])|(unsigned(p[9])<<8);
                    const auto cid=unsigned(p[10])|(unsigned(p[11])<<8);
                    QVariantMap item;
                    item["errorCount"]=QString::number(count);
                    item["submissionQueueId"]=sqid;
                    item["commandId"]=cid;
                    item["statusField"]=QStringLiteral("0x%1").arg(status,4,16,QLatin1Char('0'));
                    item["statusCode"]=(status>>1)&0xff;
                    item["statusCodeType"]=(status>>9)&0x7;
                    item["doNotRetry"]=(status&0x8000)!=0;
                    errors.append(item);
                }
                report["recentNvmeErrors"]=errors;
                report["errorLogAvailable"]=true;
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

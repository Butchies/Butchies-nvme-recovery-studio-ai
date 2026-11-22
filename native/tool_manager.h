#pragma once
// External tools the agent can use (data/catalog/tools.json).
// Detects installations, keeps user-registered paths (e.g. vendor MPTools)
// and runs smartctl to read NVMe data behind USB bridges.
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QVariantList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QDir>
#include <QStandardPaths>
#include <QProcess>
#include <QProcessEnvironment>

namespace ToolManager {

inline QString appDataFile(const QString &name) {
    const QString dir=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return QDir(dir).filePath(name);
}

inline QJsonObject loadJson(const QString &name) {
    for(const auto &path:{QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath(QStringLiteral("vorlagen/")+name),
                          QStringLiteral(":/catalog/")+name}) {
        QFile f(path);
        if(f.open(QIODevice::ReadOnly)) return QJsonDocument::fromJson(f.readAll()).object();
    }
    return {};
}

inline QString expand(QString p) {
    const auto env=QProcessEnvironment::systemEnvironment();
    for(const auto &var:{QStringLiteral("ProgramFiles(x86)"),QStringLiteral("ProgramFiles"),QStringLiteral("LOCALAPPDATA"),QStringLiteral("APPDATA")})
        p.replace(QLatin1Char('%')+var+QLatin1Char('%'),env.value(var));
    return QDir::fromNativeSeparators(p);
}

class Manager {
public:
    Manager() {
        m_tools=loadJson(QStringLiteral("tools.json")).value("werkzeuge").toArray();
        QFile f(appDataFile(QStringLiteral("werkzeuge.json")));
        if(f.open(QIODevice::ReadOnly)) m_registered=QJsonDocument::fromJson(f.readAll()).object();
    }
    QJsonObject tool(const QString &id) const {
        for(const auto &t:m_tools) if(t.toObject().value("id").toString()==id) return t.toObject();
        return {};
    }
    // Installed executable or empty.
    QString path(const QString &id) const {
        const QString reg=m_registered.value(id).toString();
        if(!reg.isEmpty() && QFileInfo::exists(reg)) return reg;
        for(const auto &p:tool(id).value("erkennung").toArray()) {
            const QString e=expand(p.toString());
            if(!e.contains(QLatin1Char('%')) && QFileInfo::exists(e)) return e;
        }
        return {};
    }
    QVariantList status() const {
        QVariantList list;
        for(const auto &t:m_tools) {
            QVariantMap m=t.toObject().toVariantMap();
            const QString p=path(m.value("id").toString());
            m["installiert"]=!p.isEmpty();
            m["pfad"]=QDir::toNativeSeparators(p);
            m["automatisch"]=t.toObject().contains("automatischerDownload");
            list.append(m);
        }
        return list;
    }
    QString signature() const {
        QString s; for(const auto &t:m_tools) s+=path(t.toObject().value("id").toString())+QLatin1Char('|'); return s;
    }
    bool registerTool(const QString &id, const QString &file, QString &error) {
        const QFileInfo fi(file);
        if(tool(id).isEmpty()) { error=QStringLiteral("Unbekanntes Werkzeug."); return false; }
        if(!fi.exists() || !fi.isFile()) { error=QStringLiteral("Datei nicht gefunden."); return false; }
        m_registered[id]=fi.absoluteFilePath();
        QSaveFile f(appDataFile(QStringLiteral("werkzeuge.json")));
        if(!f.open(QIODevice::WriteOnly) || f.write(QJsonDocument(m_registered).toJson())<0 || !f.commit()) {
            error=QStringLiteral("Registrierung konnte nicht gespeichert werden."); return false;
        }
        return true;
    }
    // Tools relevant for a controller family id (plus the general ones).
    QStringList forFamily(const QString &family) const {
        QStringList ids;
        for(const auto &t:m_tools) {
            const auto o=t.toObject();
            for(const auto &f:o.value("familien").toArray())
                if(f.toString()==family || f.toString()==QStringLiteral("alle")) { ids<<o.value("id").toString(); break; }
        }
        return ids;
    }
private:
    QJsonArray m_tools;
    QJsonObject m_registered;
};

// Reads NVMe identity, SMART/Health and error log through smartctl. Only
// read commands are used (-a = identify, health, error log, self-test log).
// Returns empty map on failure; "smartctlTyp" names the working -d type.
inline QVariantMap querySmartctl(const QString &smartctl, const QString &devicePath, const QStringList &types) {
    QVariantMap out;
    if(smartctl.isEmpty()) return out;
    bool ok=false;
    const int n=devicePath.mid(QStringLiteral("\\\\.\\PHYSICALDRIVE").size()).toInt(&ok);
    if(!ok) return out;
    const QString dev=QStringLiteral("/dev/pd%1").arg(n);
    for(const auto &type:types) {
        QProcess p;
        p.start(smartctl,{QStringLiteral("-a"),QStringLiteral("-j"),QStringLiteral("-d"),type,dev});
        if(!p.waitForFinished(25000)) { p.kill(); p.waitForFinished(2000); continue; }
        const auto j=QJsonDocument::fromJson(p.readAllStandardOutput()).object();
        const QString model=j.value("model_name").toString();
        if(model.isEmpty()) continue;
        const auto h=j.value("nvme_smart_health_information_log").toObject();
        const auto pciv=j.value("nvme_pci_vendor").toObject();
        out["smartctlTyp"]=type;
        out["nvmeIdentifyAvailable"]=true;
        out["nvmePassthrough"]=QStringLiteral("NVMe über Bridge-Befehle (smartctl -d %1)").arg(type);
        out["nvmeControllerModel"]=model;
        out["nvmeControllerSerial"]=j.value("serial_number").toString();
        out["nvmeFirmware"]=j.value("firmware_version").toString();
        if(pciv.contains("id")) {
            out["nvmeVendorId"]=QStringLiteral("%1").arg(pciv.value("id").toInt(),4,16,QLatin1Char('0'));
            out["nvmeSubsystemVendorId"]=QStringLiteral("%1").arg(pciv.value("subsystem_id").toInt(),4,16,QLatin1Char('0'));
        }
        if(!h.isEmpty()) {
            out["smartAvailable"]=true;
            out["criticalWarning"]=QStringLiteral("0x%1").arg(h.value("critical_warning").toInt(),2,16,QLatin1Char('0'));
            out["temperatureCelsius"]=QString::number(h.value("temperature").toInt());
            out["availableSparePercent"]=QString::number(h.value("available_spare").toInt());
            out["percentageUsed"]=QString::number(h.value("percentage_used").toInt());
            out["mediaErrors"]=QString::number(qint64(h.value("media_errors").toDouble()));
            out["errorLogEntries"]=QString::number(qint64(h.value("num_err_log_entries").toDouble()));
        }
        const auto errs=j.value("nvme_error_information_log").toObject().value("table").toArray();
        QVariantList errors;
        for(const auto &e:errs) {
            const auto o=e.toObject();
            if(errors.size()>=4) break;
            QVariantMap item;
            item["errorCount"]=QString::number(qint64(o.value("error_count").toDouble()));
            item["submissionQueueId"]=o.value("submission_queue_id").toInt();
            item["commandId"]=o.value("command_id").toInt();
            const int status=o.value("status_field").toObject().value("value").toInt();
            item["statusField"]=QStringLiteral("0x%1").arg(status,4,16,QLatin1Char('0'));
            item["statusCode"]=o.value("status_field").toObject().value("status_code").toInt();
            item["statusCodeType"]=o.value("status_field").toObject().value("status_code_type").toInt();
            item["doNotRetry"]=o.value("status_field").toObject().value("do_not_retry").toBool();
            item["statusOhnePhase"]=true; // smartctl reports the 15-bit status without phase tag
            if(o.contains("lba")) item["lba"]=QString::number(qint64(o.value("lba").toObject().value("value").toDouble()));
            errors.append(item);
        }
        out["recentNvmeErrors"]=errors;
        out["errorLogAvailable"]=j.contains("nvme_error_information_log");
        return out;
    }
    return out;
}

} // namespace ToolManager

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QOperatingSystemVersion>
#include <QObject>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QVariantList>
#include <QVariantMap>
#include <QFileInfo>
#include <QDir>
#include <QStorageInfo>
#include <QStandardPaths>
#include <QRegularExpression>
#include <QThread>
#include <QStringList>
#include <atomic>
#include "usb_bridge.h"
#include "recovery_agent.h"
#include "rescue_imager.h"
#include "knowledge_base.h"
#include "controller_catalog.h"
#include <QTimer>
#include <QTime>

class RecoveryController final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList devices READ devices NOTIFY devicesChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QVariantMap aiReport READ aiReport NOTIFY aiReportChanged)
    Q_PROPERTY(bool recoveryRunning READ recoveryRunning NOTIFY recoveryChanged)
    Q_PROPERTY(QVariantMap recoveryProgress READ recoveryProgress NOTIFY recoveryChanged)
    Q_PROPERTY(QVariantMap recoveryResult READ recoveryResult NOTIFY recoveryChanged)
    Q_PROPERTY(QStringList recoveryLog READ recoveryLog NOTIFY recoveryLogChanged)
    Q_PROPERTY(QStringList aiMonitor READ aiMonitor NOTIFY aiMonitorChanged)
    Q_PROPERTY(QVariantMap knowledge READ knowledge NOTIFY aiMonitorChanged)
    Q_PROPERTY(QVariantMap catalog READ catalog CONSTANT)
public:
    explicit RecoveryController(QObject *parent=nullptr):QObject(parent) {
        const auto k=m_kb.summary();
        ai(QStringLiteral("KI-Agent aktiv. Wissensbasis: %1 Einträge, %2 Geräte, %3 USB-Bridges, %4 Controller-Modelle.")
            .arg(k.value("cases").toInt()).arg(k.value("devices").toInt()).arg(k.value("bridges").toInt()).arg(k.value("controllers").toInt()));
        const auto c=ControllerCatalog::totals();
        ai(QStringLiteral("Vorlagen geladen: %1 Hersteller, %2 NVMe-Controller/-Produkte (pci.ids %3), %4 Controllerfamilien, %5 bekannte Firmwarefehler, %6 USB-Bridges.")
            .arg(c.value("vendors").toInt()).arg(c.value("devices").toInt()).arg(c.value("pciVersion").toString())
            .arg(c.value("families").toInt()).arg(c.value("firmwareIssues").toInt()).arg(c.value("bridges").toInt()));
        ai(QStringLiteral("Überwache USB-Anschlüsse: neue SSDs werden automatisch erkannt und untersucht."));
        connect(&m_watch,&QTimer::timeout,this,&RecoveryController::checkDevices);
        m_watch.start(4000);
    }
    QStringList aiMonitor() const { return m_aiLog; }
    QVariantMap knowledge() const { return m_kb.summary(); }
    QVariantMap catalog() const { return ControllerCatalog::totals(); }
    ~RecoveryController() override {
        m_cancel=true;
        if(m_worker) { m_worker->wait(); }
    }
    bool recoveryRunning() const { return m_worker!=nullptr; }
    QVariantMap recoveryProgress() const { return m_progress; }
    QVariantMap recoveryResult() const { return m_result; }
    QStringList recoveryLog() const { return m_log; }
    QVariantList devices() const { return m_devices; }
    QString status() const { return m_status; }
    QVariantMap aiReport() const { return m_aiReport; }
    Q_INVOKABLE void refresh() {
        if(m_worker) { setStatus(QStringLiteral("Rettung läuft; Geräteliste wird währenddessen nicht neu eingelesen.")); return; }
#ifdef Q_OS_WIN
        QProcess ps;
        ps.start(QStringLiteral("powershell.exe"), {
            QStringLiteral("-NoProfile"),QStringLiteral("-NonInteractive"),
            QStringLiteral("-Command"),
            QStringLiteral("$ErrorActionPreference='Stop'; Get-CimInstance Win32_DiskDrive | Where-Object { $_.InterfaceType -eq 'USB' -or $_.PNPDeviceID -match '^USBSTOR|^USB\\' } | Select-Object Model,SerialNumber,Size,DeviceID,InterfaceType,PNPDeviceID | ConvertTo-Json -Compress")
        });
        if (!ps.waitForFinished(15000) || ps.exitStatus()!=QProcess::NormalExit || ps.exitCode()!=0) {
            setStatus(QStringLiteral("USB-Inventarisierung fehlgeschlagen; keine Geräte freigegeben."));
            m_devices.clear(); emit devicesChanged(); return;
        }
        const auto doc=QJsonDocument::fromJson(ps.readAllStandardOutput());
        QJsonArray items;
        if(doc.isArray()) items=doc.array();
        else if(doc.isObject()) items.append(doc.object());
        QVariantList found;
        for(const auto &item:items) {
            if(!item.isObject()) continue;
            const auto obj=item.toObject();
            const QString path=obj.value("DeviceID").toString();
            if(!path.startsWith(QStringLiteral("\\\\.\\PHYSICALDRIVE"),Qt::CaseInsensitive)) continue;
            bool valid=false;
            const auto number=path.mid(QStringLiteral("\\\\.\\PHYSICALDRIVE").size()).toUInt(&valid);
            Q_UNUSED(number);
            if(!valid) continue;
            const QString pnp=obj.value("PNPDeviceID").toString();
            if(!pnp.startsWith("USB",Qt::CaseInsensitive)) continue;
            QVariantMap d;
            d["name"]=obj.value("Model").toString();
            d["serial"]=obj.value("SerialNumber").toString();
            d["sizeBytes"]=obj.value("Size").toVariant().toString();
            d["connection"]="USB";
            d["devicePath"]=path;
            d["pnpDeviceId"]=pnp;
            const QRegularExpression vidPattern(QStringLiteral("VID_([0-9A-Fa-f]{4})"));
            const QRegularExpression pidPattern(QStringLiteral("PID_([0-9A-Fa-f]{4})"));
            const auto vid=vidPattern.match(pnp);
            const auto pid=pidPattern.match(pnp);
            d["bridgeVendorId"]=vid.hasMatch() ? vid.captured(1).toUpper() : QStringLiteral("Unbekannt");
            d["bridgeProductId"]=pid.hasMatch() ? pid.captured(1).toUpper() : QStringLiteral("Unbekannt");
            const auto bridge=UsbBridge::inspect(path);
            if(bridge.value("usbConfirmed").toBool()!=true) continue;
            d["storageBus"]=bridge.value("storageBus");
            d["deviceVendor"]=bridge.value("deviceVendor");
            d["deviceProduct"]=bridge.value("deviceProduct");
            d["bridgeFirmware"]=bridge.value("deviceFirmware");
            d["bridgeInspectionError"]=bridge.value("error");
            d["nvmeIdentifyAvailable"]=bridge.value("nvmeIdentifyAvailable");
            d["nvmeControllerModel"]=bridge.value("nvmeControllerModel");
            d["nvmeControllerSerial"]=bridge.value("nvmeControllerSerial");
            d["nvmeFirmware"]=bridge.value("nvmeFirmware");
            d["smartAvailable"]=bridge.value("smartAvailable");
            d["temperatureCelsius"]=bridge.value("temperatureCelsius");
            d["criticalWarning"]=bridge.value("criticalWarning");
            d["availableSparePercent"]=bridge.value("availableSparePercent");
            d["percentageUsed"]=bridge.value("percentageUsed");
            d["mediaErrors"]=bridge.value("mediaErrors");
            d["errorLogEntries"]=bridge.value("errorLogEntries");
            d["errorLogAvailable"]=bridge.value("errorLogAvailable");
            d["recentNvmeErrors"]=bridge.value("recentNvmeErrors");
            d["nvmePassthrough"]=bridge.value("nvmePassthrough");
            d["nvmeVerified"]=bridge.value("nvmeIdentifyAvailable").toBool(); // USB bridge cannot prove underlying NVMe type.
            d["readOnly"]=true;
            found.append(d);
        }
        m_devices=found;
        m_aiReport.clear();
        emit aiReportChanged();
        emit devicesChanged();
        m_signature=UsbBridge::usbDiskSignature();
        setStatus(QStringLiteral("%1 externe USB-Geräte erkannt. Interne Laufwerke ausgeschlossen.").arg(found.size()));
        ai(found.isEmpty() ? QStringLiteral("Keine externe USB-SSD angeschlossen. Warte auf Gerät …")
                           : QStringLiteral("%1 externe USB-Geräte erkannt – starte automatische Diagnose.").arg(found.size()));
        // The agent examines every detected device on its own.
        for(int i=0;i<m_devices.size();++i) diagnose(i);
#else
        m_devices.clear();emit devicesChanged();
        setStatus(QStringLiteral("Diese native Edition ist für Windows 10/11 vorgesehen."));
#endif
    }
    Q_INVOKABLE void autoDiagnose(int index) { diagnose(index); }
    // Fully automatic sequence: fresh diagnosis -> rescue plan -> rescue
    // imaging -> verification -> evaluation -> report. Only reads the SSD.
    Q_INVOKABLE void startAutoRecovery(int index, const QString &destination) {
        if(m_worker) { setStatus(QStringLiteral("Es läuft bereits eine Rettung.")); return; }
        if(!diagnose(index)) return;
        const QVariantMap plan=RecoveryAgent::planRescue(m_aiReport,m_experience);
        if(!plan.value("allowed").toBool()) {
            setStatus(plan.value("reason").toString()); return;
        }
        if(destination.trimmed().isEmpty()) {
            setStatus(QStringLiteral("Diagnose fertig. Für die automatische Rettung bitte einen Zielordner auf einem anderen Laufwerk angeben."));
            ai(QStringLiteral("Rettung vorbereitet, aber kein Zielordner angegeben."));
            return;
        }
        for(const auto &d:plan.value("decisions").toList()) ai(QStringLiteral("Entscheidung: %1").arg(d.toString()));
        if(plan.value("learnedAdjustments").toInt()>0)
            ai(QStringLiteral("%1 Anpassungen des Rettungsplans aus früheren Versuchen übernommen.").arg(plan.value("learnedAdjustments").toInt()));
        m_kb.recordAction(QStringLiteral("rettung_gestartet"),m_verified,{{"severity",m_aiReport.value("severity")},{"blockBytes",plan.value("chunkBytes")}});
        RescueImager::Config cfg;
        cfg.sourcePath=m_verified.value("devicePath").toString();
        cfg.destinationDir=QDir::fromNativeSeparators(destination.trimmed());
        cfg.device=m_verified;
        cfg.diagnosis=m_aiReport;
        cfg.plan=plan;
        m_plan=plan;
        m_cancel=false;
        m_progress.clear();
        m_result.clear();
        m_log.clear();
        emit recoveryLogChanged();
        m_worker=QThread::create([this,cfg]{
            const auto result=RescueImager::run(cfg,m_cancel,[this](const QVariantMap &p){
                QMetaObject::invokeMethod(this,[this,p]{ onProgress(p); },Qt::QueuedConnection);
            });
            QMetaObject::invokeMethod(this,[this,result]{ onFinished(result); },Qt::QueuedConnection);
        });
        emit recoveryChanged();
        setStatus(QStringLiteral("Automatische Rettung gestartet: %1").arg(m_aiReport.value("diagnosis").toString()));
        m_worker->start();
    }
    Q_INVOKABLE void cancelRecovery() {
        if(!m_worker) return;
        m_cancel=true;
        m_kb.recordAction(QStringLiteral("rettung_angehalten_vom_benutzer"),m_verified);
        ai(QStringLiteral("Benutzer hält die Rettung an; Fortschritt wird gespeichert."));
        setStatus(QStringLiteral("Rettung wird angehalten; Fortschritt wird gespeichert …"));
    }
private:
    bool diagnose(int index) {
        if(index<0 || index>=m_devices.size()) {
            m_aiReport.clear();
            emit aiReportChanged();
            setStatus(QStringLiteral("KI-Diagnose: Kein externes USB-Gerät ausgewählt."));
            return false;
        }
        const auto device=m_devices.at(index).toMap();
        // Fresh hardware query, not stale inventory data.
        const auto fresh=UsbBridge::inspect(device.value("devicePath").toString());
        if(!fresh.value("usbConfirmed").toBool()) {
            m_aiReport.clear();
            emit aiReportChanged();
            setStatus(QStringLiteral("KI-Diagnose gestoppt: USB-Gerät nicht mehr bestätigt."));
            return false;
        }
        QVariantMap verified=device;
        for(auto it=fresh.constBegin();it!=fresh.constEnd();++it)
            verified[it.key()]=it.value();
        verified["connection"]="USB";
        verified["readOnly"]=true;
        m_verified=verified;
        m_experience=m_kb.experience(verified);
        m_aiReport=RecoveryAgent::analyze(verified,m_experience);
        m_kb.recordDiagnosis(verified,m_aiReport);
        {
            auto entry=m_devices[index].toMap();
            entry["aiDiagnosis"]=m_aiReport.value("diagnosis");
            entry["aiSeverity"]=m_aiReport.value("severity");
            m_devices[index]=entry;
            emit devicesChanged();
        }
        ai(QStringLiteral("Diagnose %1: %2 [%3]").arg(verified.value("name").toString(),
            m_aiReport.value("diagnosis").toString(),m_aiReport.value("severity").toString()));
        for(const auto &e:m_aiReport.value("evidence").toList()) ai(QStringLiteral("Beleg: %1").arg(e.toString()));
        for(const auto &w:m_aiReport.value("warnings").toList()) ai(QStringLiteral("Warnung: %1").arg(w.toString()));
        for(const auto &p:m_aiReport.value("repairPaths").toList()) {
            const auto m=p.toMap();
            ai(QStringLiteral("Reparaturweg %1: %2").arg(m.value("typ").toString(),m.value("status").toString()));
        }
        if(m_aiReport.value("learnedFindings").toInt()>0)
            ai(QStringLiteral("Erfahrung genutzt: %1 Erkenntnisse aus früheren Fällen.").arg(m_aiReport.value("learnedFindings").toInt()));
        emit aiReportChanged();
        setStatus(QStringLiteral("Automatische Diagnose abgeschlossen: %1")
            .arg(m_aiReport.value("diagnosis").toString()));
        return true;
    }
    void onProgress(const QVariantMap &p) {
        if(p.contains("event")) {
            m_log.append(p.value("event").toString());
            while(m_log.size()>300) m_log.removeFirst();
            emit recoveryLogChanged();
            return;
        }
        for(auto it=p.constBegin();it!=p.constEnd();++it) m_progress[it.key()]=it.value();
        emit recoveryChanged();
    }
    void onFinished(const QVariantMap &result) {
        if(m_worker) { m_worker->wait(); m_worker->deleteLater(); m_worker=nullptr; }
        m_result=result;
        const auto evaluation=RecoveryAgent::evaluateRescue(m_aiReport,result);
        m_result["summary"]=evaluation.value("summary");
        m_result["rating"]=evaluation.value("rating");
        m_result["nextSteps"]=evaluation.value("nextSteps");
        const QString report=RescueImager::writeReport(result.value("folder").toString(),
            m_verified,m_aiReport,m_plan,result,evaluation);
        m_result["reportPath"]=QDir::toNativeSeparators(report);
        m_kb.recordRescue(m_verified,m_plan,result);
        ai(QStringLiteral("Ergebnis: %1").arg(evaluation.value("summary").toString()));
        for(const auto &n:evaluation.value("nextSteps").toList()) ai(QStringLiteral("Empfehlung: %1").arg(n.toString()));
        {
            const auto exp=m_kb.experience(m_verified);
            const auto br=exp.value("bridge").toMap();
            ai(QStringLiteral("Gelernt und gespeichert: Bridge %1 jetzt %2 Rettungen (%3 Zeitüberschreitungen, %4 Verbindungsabbrüche). Wird bei der nächsten Rettung berücksichtigt.")
                .arg(exp.value("bridgeKey").toString()).arg(br.value("rettungen").toInt())
                .arg(br.value("zeitueberschreitungen").toInt()).arg(br.value("verbindungsverluste").toInt()));
        }
        emit recoveryChanged();
        setStatus(report.isEmpty() ? evaluation.value("summary").toString()
            : QStringLiteral("%1 Bericht: %2").arg(evaluation.value("summary").toString(),QDir::toNativeSeparators(report)));
    }
    void checkDevices() {
        if(m_worker) return;
        const QString sig=UsbBridge::usbDiskSignature();
        if(sig==m_signature) return;
        ai(QStringLiteral("Änderung an den USB-Laufwerken erkannt – KI untersucht automatisch."));
        refresh();
        m_signature=sig;
    }
    void ai(const QString &text) {
        m_aiLog.append(QStringLiteral("%1  %2").arg(QTime::currentTime().toString(QStringLiteral("HH:mm:ss")),text));
        while(m_aiLog.size()>400) m_aiLog.removeFirst();
        emit aiMonitorChanged();
    }
public:
    Q_INVOKABLE void selectDevice(int index) {
        if(index<0 || index>=m_devices.size()) {
            setStatus(QStringLiteral("Keine gültige USB-Geräteauswahl."));
            return;
        }
        const auto device=m_devices.at(index).toMap();
        setStatus(QStringLiteral("%1 | USB VID:%2 PID:%3 | Revision:%4 | %5")
            .arg(device.value("name").toString(),
                 device.value("bridgeVendorId").toString(),
                 device.value("bridgeProductId").toString(),
                 device.value("bridgeFirmware").toString(),
                 device.value("nvmePassthrough").toString()));
        if(device.value("errorLogAvailable").toBool() &&
           !device.value("recentNvmeErrors").toList().isEmpty()) {
            const auto entry=device.value("recentNvmeErrors").toList().first().toMap();
            setStatus(QStringLiteral("%1 | NVMe-Fehlerzähler %2 | Status %3 | SCT %4")
                .arg(device.value("name").toString(),
                     entry.value("errorCount").toString(),
                     entry.value("statusField").toString(),
                     entry.value("statusCodeType").toString()));
        } else if(device.value("smartAvailable").toBool()) {
            setStatus(QStringLiteral("%1 | Temperatur: %2 °C | Reserve: %3 %% | Verschleiß: %4 %% | Medienfehler: %5")
                .arg(device.value("name").toString(),
                     device.value("temperatureCelsius").toString(),
                     device.value("availableSparePercent").toString(),
                     device.value("percentageUsed").toString(),
                     device.value("mediaErrors").toString()));
        }
    }
    Q_INVOKABLE void inspectDestination(const QString &path) {
        const QFileInfo info(path);
        if(!info.exists() || !info.isDir() || info.isSymLink()) {
            setStatus(QStringLiteral("Ungültiges Zielverzeichnis.")); return;
        }
        const QStorageInfo volume(path);
        if(!volume.isValid() || !volume.isReady() || volume.isReadOnly()) {
            setStatus(QStringLiteral("Ziellaufwerk nicht beschreibbar."));return;
        }
        m_kb.recordAction(QStringLiteral("ziel_geprueft"));
        setStatus(QStringLiteral("Ziel verfügbar: %1 GiB frei. Noch keine Recovery gestartet.")
            .arg(volume.bytesAvailable()/1024/1024/1024));
    }
signals:
    void devicesChanged();
    void statusChanged();
    void aiReportChanged();
    void recoveryChanged();
    void recoveryLogChanged();
    void aiMonitorChanged();
private:
    void setStatus(const QString &text){m_status=text;emit statusChanged();}
    QVariantList m_devices;
    QVariantMap m_aiReport;
    QVariantMap m_verified;
    QVariantMap m_plan;
    QVariantMap m_progress;
    QVariantMap m_result;
    QStringList m_log;
    QThread *m_worker=nullptr;
    std::atomic_bool m_cancel{false};
    KnowledgeBase::Store m_kb;
    QVariantMap m_experience;
    QStringList m_aiLog;
    QTimer m_watch;
    QString m_signature;
    QString m_status=QStringLiteral("Bereit. Keine Schreibzugriffe auf SSDs.");
};
int main(int argc,char *argv[]) {
    QGuiApplication app(argc,argv);
    RecoveryController controller;
    QQmlApplicationEngine engine;
    engine.setInitialProperties({{"controller",QVariant::fromValue(&controller)}});
    engine.loadFromModule("RecoveryStudio","Main");
    if(engine.rootObjects().isEmpty()) return -1;
    controller.refresh();
    return app.exec();
}
#include "main.moc"

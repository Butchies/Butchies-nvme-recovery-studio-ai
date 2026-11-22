#pragma once
// Local, persistent experience store of the recovery agent.
//
// Every diagnosis, every rescue attempt and every user action is recorded.
// Experience is aggregated per device (hashed serial), per USB bridge
// (VID:PID) and per controller model. RecoveryAgent uses this experience to
// detect trends and to adapt future rescue plans. Stored locally only
// (AppData), no network transfer. Serial numbers are stored as hashes.
#include <QString>
#include <QVariantMap>
#include <QVariantList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QSaveFile>
#include <QDir>
#include <QDateTime>
#include <QStandardPaths>
#include <QCryptographicHash>

namespace KnowledgeBase {

inline QString now() { return QDateTime::currentDateTime().toString(Qt::ISODate); }

inline bool known(const QVariant &v) {
    const QString s=v.toString().trimmed();
    return !s.isEmpty() && s!=QStringLiteral("Nicht ermittelt") && s!=QStringLiteral("Unbekannt");
}

class Store {
public:
    Store() {
        const QString dir=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dir);
        m_path=QDir(dir).filePath(QStringLiteral("ki-wissen.json"));
        QFile f(m_path);
        if(f.open(QIODevice::ReadOnly)) m_root=QJsonDocument::fromJson(f.readAll()).object();
        if(m_root.value("version").toInt()!=1) {
            m_root=QJsonObject{{"version",1},{"angelegt",now()}};
        }
    }
    QString path() const { return m_path; }

    static QString deviceKey(const QVariantMap &d) {
        QString id=known(d.value("nvmeControllerSerial")) ? d.value("nvmeControllerSerial").toString()
                                                           : d.value("serial").toString();
        id=id.trimmed()+QLatin1Char('|')+d.value("nvmeControllerModel").toString().trimmed()
           +QLatin1Char('|')+d.value("name").toString().trimmed();
        return QString::fromLatin1(QCryptographicHash::hash(id.toUtf8(),QCryptographicHash::Sha256).toHex().left(16));
    }
    static QString bridgeKey(const QVariantMap &d) {
        return QStringLiteral("%1:%2").arg(d.value("bridgeVendorId").toString(),d.value("bridgeProductId").toString());
    }
    static QString controllerKey(const QVariantMap &d) {
        return known(d.value("nvmeControllerModel")) ? d.value("nvmeControllerModel").toString().trimmed() : QString();
    }

    // Aggregated experience for one device, its bridge and controller model.
    QVariantMap experience(const QVariantMap &d) const {
        QVariantMap e;
        e["totalCases"]=m_root.value("attempts").toArray().size();
        const auto dev=m_root.value("devices").toObject().value(deviceKey(d)).toObject();
        e["device"]=dev.toVariantMap();
        e["bridgeKey"]=bridgeKey(d);
        e["bridge"]=m_root.value("bridges").toObject().value(bridgeKey(d)).toObject().toVariantMap();
        const QString ck=controllerKey(d);
        e["controllerKey"]=ck;
        if(!ck.isEmpty()) e["controller"]=m_root.value("controllers").toObject().value(ck).toObject().toVariantMap();
        return e;
    }
    QVariantMap summary() const {
        return {
            {"cases",m_root.value("attempts").toArray().size()},
            {"devices",m_root.value("devices").toObject().size()},
            {"bridges",m_root.value("bridges").toObject().size()},
            {"controllers",m_root.value("controllers").toObject().size()},
            {"proposals",m_root.value("vorlagenVorschlaege").toObject().size()},
        };
    }

    void recordAction(const QString &action, const QVariantMap &device=QVariantMap(), const QVariantMap &details=QVariantMap()) {
        QJsonObject a{{"zeit",now()},{"aktion",action}};
        if(!device.isEmpty()) {
            a["geraet"]=deviceKey(device);
            a["bridge"]=bridgeKey(device);
            a["controller"]=controllerKey(device);
        }
        if(!details.isEmpty()) a["details"]=QJsonObject::fromVariantMap(details);
        auto list=m_root.value("attempts").toArray();
        list.append(a);
        while(list.size()>5000) list.removeFirst();
        m_root["attempts"]=list;
        save();
    }

    void recordDiagnosis(const QVariantMap &d, const QVariantMap &report) {
        // Device history with SMART snapshot for trend detection.
        auto devices=m_root.value("devices").toObject();
        auto dev=devices.value(deviceKey(d)).toObject();
        if(!dev.contains("ersteUntersuchung")) dev["ersteUntersuchung"]=now();
        dev["letzteUntersuchung"]=now();
        dev["diagnosen"]=dev.value("diagnosen").toInt()+1;
        dev["modell"]=d.value("nvmeControllerModel").toString();
        dev["letzteBewertung"]=report.value("severity").toString();
        if(d.value("smartAvailable").toBool()) {
            QJsonObject smart{
                {"zeit",now()},
                {"reserve",d.value("availableSparePercent").toString()},
                {"verschleiss",d.value("percentageUsed").toString()},
                {"medienfehler",d.value("mediaErrors").toString()},
                {"fehlerprotokoll",d.value("errorLogEntries").toString()},
                {"warnbits",d.value("criticalWarning").toString()},
                {"temperatur",d.value("temperatureCelsius").toString()},
            };
            dev["letzterSmart"]=smart;
            auto hist=dev.value("smartVerlauf").toArray();
            hist.append(smart);
            while(hist.size()>50) hist.removeFirst();
            dev["smartVerlauf"]=hist;
        }
        devices[deviceKey(d)]=dev;
        m_root["devices"]=devices;

        auto bridges=m_root.value("bridges").toObject();
        auto br=bridges.value(bridgeKey(d)).toObject();
        br["faelle"]=br.value("faelle").toInt()+1;
        if(d.value("nvmeIdentifyAvailable").toBool()) br["identifyOk"]=br.value("identifyOk").toInt()+1;
        if(d.value("smartAvailable").toBool()) br["smartOk"]=br.value("smartOk").toInt()+1;
        br["produkt"]=d.value("deviceProduct").toString();
        bridges[bridgeKey(d)]=br;
        m_root["bridges"]=bridges;

        const QString ck=controllerKey(d);
        if(!ck.isEmpty()) {
            auto controllers=m_root.value("controllers").toObject();
            auto c=controllers.value(ck).toObject();
            c["faelle"]=c.value("faelle").toInt()+1;
            if(report.value("severity").toString()==QStringLiteral("Kritisch")) c["kritisch"]=c.value("kritisch").toInt()+1;
            auto fw=c.value("firmware").toObject();
            const QString f=d.value("nvmeFirmware").toString();
            fw[f]=fw.value(f).toInt()+1;
            c["firmware"]=fw;
            controllers[ck]=c;
            m_root["controllers"]=controllers;
        }
        // Template gaps: unknown vendor or controller become proposals in
        // quarantine. They never unlock write operations by themselves.
        const auto cat=report.value("catalog").toMap();
        if(d.value("nvmeIdentifyAvailable").toBool() && (cat.value("unknownVendor").toBool() || !cat.contains("controllerName"))) {
            auto proposals=m_root.value("vorlagenVorschlaege").toObject();
            const QString key=QStringLiteral("%1|%2").arg(cat.value("vendorId").toString(),d.value("nvmeControllerModel").toString());
            auto prop=proposals.value(key).toObject();
            prop["status"]="quarantaene";
            prop["pciVendor"]=cat.value("vendorId").toString();
            prop["modell"]=d.value("nvmeControllerModel").toString();
            prop["firmware"]=d.value("nvmeFirmware").toString();
            prop["bridge"]=bridgeKey(d);
            prop["beobachtungen"]=prop.value("beobachtungen").toInt()+1;
            if(!prop.contains("erstmals")) prop["erstmals"]=now();
            proposals[key]=prop;
            m_root["vorlagenVorschlaege"]=proposals;
        }
        recordAction(QStringLiteral("diagnose"),d,{{"bewertung",report.value("severity")},{"diagnose",report.value("diagnosis")}});
    }

    void recordRescue(const QVariantMap &d, const QVariantMap &plan, const QVariantMap &result) {
        const QString state=result.value("state").toString();
        if(state==QStringLiteral("fehler")) {
            recordAction(QStringLiteral("rettung_nicht_gestartet"),d,{{"grund",result.value("message")}});
            return;
        }
        QJsonObject last{
            {"zeit",now()},
            {"status",state},
            {"prozent",result.value("rescuedPercent").toDouble()},
            {"defektBytes",result.value("badBytes").toString()},
            {"defektBereiche",result.value("badRegions").toInt()},
            {"lesefehler",result.value("readErrors").toInt()},
            {"zeitueberschreitungen",result.value("timeouts").toInt()},
            {"verbindungsverluste",result.value("deviceLost").toInt()},
            {"rateBytesProSek",result.value("rateBytesPerSec").toDouble()},
            {"blockBytes",plan.value("chunkBytes").toLongLong()},
            {"zeitlimitMs",plan.value("readTimeoutMs").toInt()},
        };
        auto devices=m_root.value("devices").toObject();
        auto dev=devices.value(deviceKey(d)).toObject();
        dev["rettungen"]=dev.value("rettungen").toInt()+1;
        dev["letzteRettung"]=last;
        devices[deviceKey(d)]=dev;
        m_root["devices"]=devices;

        auto bridges=m_root.value("bridges").toObject();
        auto br=bridges.value(bridgeKey(d)).toObject();
        br["rettungen"]=br.value("rettungen").toInt()+1;
        br["zeitueberschreitungen"]=br.value("zeitueberschreitungen").toInt()+result.value("timeouts").toInt();
        br["verbindungsverluste"]=br.value("verbindungsverluste").toInt()+result.value("deviceLost").toInt();
        const double rate=result.value("rateBytesPerSec").toDouble();
        if(rate>0) {
            br["rateSumme"]=br.value("rateSumme").toDouble()+rate;
            br["rateMessungen"]=br.value("rateMessungen").toInt()+1;
        }
        bridges[bridgeKey(d)]=br;
        m_root["bridges"]=bridges;

        const QString ck=controllerKey(d);
        if(!ck.isEmpty()) {
            auto controllers=m_root.value("controllers").toObject();
            auto c=controllers.value(ck).toObject();
            c["rettungen"]=c.value("rettungen").toInt()+1;
            if(state==QStringLiteral("abgeschlossen")) {
                if(result.value("badBytes").toULongLong()==0) c["vollstaendig"]=c.value("vollstaendig").toInt()+1;
                else c["teilweise"]=c.value("teilweise").toInt()+1;
            } else c["unterbrochen"]=c.value("unterbrochen").toInt()+1;
            c["prozentSumme"]=c.value("prozentSumme").toDouble()+result.value("rescuedPercent").toDouble();
            controllers[ck]=c;
            m_root["controllers"]=controllers;
        }
        recordAction(QStringLiteral("rettung"),d,last.toVariantMap());
    }

    void setBoardLayout(const QVariantMap &d, const QString &id) {
        auto devices=m_root.value("devices").toObject();
        auto dev=devices.value(deviceKey(d)).toObject();
        dev["platinenLayout"]=id;
        devices[deviceKey(d)]=dev;
        m_root["devices"]=devices;
        recordAction(QStringLiteral("platinen_layout"),d,{{"layout",id}});
    }
    void recordCacheTest(const QVariantMap &d, const QVariantMap &eval) {
        auto devices=m_root.value("devices").toObject();
        auto dev=devices.value(deviceKey(d)).toObject();
        dev["letzterCacheTest"]=QJsonObject{{"zeit",now()},{"urteil",eval.value("urteil").toString()},
            {"auffaellig",eval.value("auffaellig").toBool()},{"selbsttestFehlgeschlagen",eval.value("selbsttestFehlgeschlagen").toBool()},
            {"cacheNichtWirksam",eval.value("cacheNichtWirksam").toBool()},{"latenzInstabil",eval.value("latenzInstabil").toBool()}};
        devices[deviceKey(d)]=dev;
        m_root["devices"]=devices;
        recordAction(QStringLiteral("cache_test"),d,{{"urteil",eval.value("urteil")}});
    }
    // Learned: which bridge protocol (smartctl -d type) works for a bridge.
    void recordBridgeProtocol(const QVariantMap &d, const QString &type) {
        auto bridges=m_root.value("bridges").toObject();
        auto br=bridges.value(bridgeKey(d)).toObject();
        br["smartctlTyp"]=type;
        bridges[bridgeKey(d)]=br;
        m_root["bridges"]=bridges;
        recordAction(QStringLiteral("bridge_protokoll_gelernt"),d,{{"smartctlTyp",type}});
    }
private:
    void save() {
        QSaveFile f(m_path);
        if(f.open(QIODevice::WriteOnly)) { f.write(QJsonDocument(m_root).toJson(QJsonDocument::Compact)); f.commit(); }
    }
    QString m_path;
    QJsonObject m_root;
};

} // namespace KnowledgeBase

#pragma once
// Controller, bridge and firmware templates for the recovery agent.
//
// Data comes from data/catalog/*.json (embedded as Qt resources under
// :/catalog/). A file of the same name in <AppData>/vorlagen/ overrides the
// embedded copy, so templates can be updated without rebuilding.
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QVariantList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QRegularExpression>

namespace ControllerCatalog {

inline QString normalize(QString s) {
    s=s.toUpper();
    s.remove(QRegularExpression(QStringLiteral("[^A-Z0-9]")));
    return s;
}

class Catalog {
public:
    static const Catalog &instance() { static const Catalog c; return c; }

    QJsonObject pci, usbStorage, usbNvme, families, firmware;
    QStringList sources;

    QVariantMap totals() const {
        int devices=0;
        const auto vendors=pci.value("vendors").toObject();
        for(auto it=vendors.begin();it!=vendors.end();++it) devices+=it.value().toObject().value("devices").toObject().size();
        return {
            {"vendors",vendors.size()},
            {"devices",devices},
            {"families",families.value("familien").toArray().size()},
            {"firmwareIssues",firmware.value("eintraege").toArray().size()},
            {"bridges",usbNvme.value("bridges").toArray().size()+usbStorage.value("devices").toObject().size()},
            {"pciVersion",pci.value("version").toString()},
        };
    }

    // Full classification of one device.
    QVariantMap identify(const QVariantMap &d) const {
        QVariantMap r;
        QVariantList evidence;
        const QString model=d.value("nvmeControllerModel").toString();
        const QString nmodel=normalize(model==QStringLiteral("Nicht ermittelt") ? d.value("name").toString() : model);
        const QString firmwareRev=d.value("nvmeFirmware").toString().trimmed();

        // --- USB bridge -----------------------------------------------------
        const QString usbKey=QStringLiteral("%1:%2").arg(d.value("bridgeVendorId").toString(),d.value("bridgeProductId").toString()).toLower();
        bool bridgeFound=false;
        for(const auto &b:usbNvme.value("bridges").toArray()) {
            const auto o=b.toObject();
            if(o.value("usbIds").toObject().contains(usbKey)) {
                QVariantMap bridge=o.toVariantMap();
                bridge["idVertrauen"]=o.value("usbIds").toObject().value(usbKey).toString();
                r["bridge"]=bridge;
                evidence.append(QStringLiteral("USB-Bridge erkannt: %1 (%2). NVMe-Durchreichung: %3.")
                    .arg(o.value("chip").toString(),usbKey,o.value("nvmePassthrough").toString()));
                bridgeFound=true;
                break;
            }
        }
        if(!bridgeFound) {
            const auto e=usbStorage.value("devices").toObject().value(usbKey).toObject();
            if(!e.isEmpty()) {
                r["bridge"]=e.toVariantMap();
                evidence.append(QStringLiteral("USB-Gerät laut usb.ids: %1 %2.").arg(e.value("vendor").toString(),e.value("name").toString()));
                if(e.value("typ").toString()==QStringLiteral("sata"))
                    r["bridgeIsSata"]=true;
            }
        }

        // --- Controller vendor ------------------------------------------------
        QString vid=d.value("nvmeVendorId").toString().toLower();
        if(vid.isEmpty() || vid==QStringLiteral("0000")) vid=d.value("pciVendorId").toString().toLower();
        const auto vendors=pci.value("vendors").toObject();
        const auto vendor=vendors.value(vid).toObject();
        if(!vid.isEmpty() && !vendor.isEmpty()) {
            r["vendorId"]=vid;
            r["vendorName"]=vendor.value("name").toString();
            evidence.append(QStringLiteral("Controller-Hersteller laut NVMe-Identify (PCI-Vendor %1): %2.").arg(vid,vendor.value("name").toString()));
        } else if(!vid.isEmpty()) {
            r["vendorId"]=vid;
            r["unknownVendor"]=true;
            evidence.append(QStringLiteral("PCI-Vendor %1 ist in den Vorlagen nicht als SSD-Hersteller verzeichnet.").arg(vid));
        }

        // --- Controller / product -----------------------------------------
        const QString did=d.value("pciDeviceId").toString().toLower();
        if(!did.isEmpty() && vendor.value("devices").toObject().contains(did)) {
            const auto dev=vendor.value("devices").toObject().value(did).toObject();
            r["controllerName"]=dev.value("name").toString();
            r["controllerMatch"]=QStringLiteral("PCI-ID %1:%2 (exakt)").arg(vid,did);
        } else if(!nmodel.isEmpty()) {
            // Match model name against device/subsystem names (of this vendor first).
            QString best, bestToken, bestId;
            const auto scan=[&](const QString &vId,const QJsonObject &devs) {
                for(auto it=devs.begin();it!=devs.end();++it) {
                    const auto dev=it.value().toObject();
                    QStringList names{dev.value("name").toString()};
                    const auto subs=dev.value("subsystems").toObject();
                    for(auto s=subs.begin();s!=subs.end();++s) names<<s.value().toString();
                    for(const auto &n:names) {
                        for(const auto &tok:n.split(QRegularExpression(QStringLiteral("[\\s/,()\\[\\]]+")),Qt::SkipEmptyParts)) {
                            const QString t=normalize(tok);
                            if(t.size()<4 || !t.contains(QRegularExpression(QStringLiteral("[0-9]"))) || !t.contains(QRegularExpression(QStringLiteral("[A-Z]")))) continue;
                            if(nmodel.contains(t) && t.size()>bestToken.size()) {
                                bestToken=t; best=dev.value("name").toString(); bestId=vId+QLatin1Char(':')+it.key();
                            }
                        }
                    }
                }
            };
            if(!vendor.isEmpty()) scan(vid,vendor.value("devices").toObject());
            if(bestToken.isEmpty() && vendor.isEmpty())
                for(auto v=vendors.begin();v!=vendors.end();++v) scan(v.key(),v.value().toObject().value("devices").toObject());
            if(!bestToken.isEmpty()) {
                r["controllerName"]=best;
                r["controllerMatch"]=QStringLiteral("Modellname enthält „%1“ → %2 (Namensabgleich, nicht exakt)").arg(bestToken,bestId);
                if(r.value("vendorName").toString().isEmpty())
                    r["vendorName"]=vendors.value(bestId.left(4)).toObject().value("name").toString();
                if(vid.isEmpty()) vid=bestId.left(4);
            }
        }
        if(r.contains("controllerName"))
            evidence.append(QStringLiteral("Controller/Produkt: %1 – %2.").arg(r.value("controllerName").toString(),r.value("controllerMatch").toString()));

        // --- Family knowledge -------------------------------------------------
        for(const auto &f:families.value("familien").toArray()) {
            const auto o=f.toObject();
            bool hit=false; QString note;
            for(const auto &v:o.value("pciVendor").toArray()) if(v.toString()==vid) hit=true;
            const auto rel=o.value("verwandtePciVendor").toObject();
            if(!hit && rel.contains(vid)) { hit=true; note=rel.value(vid).toString(); }
            if(!hit) continue;
            QVariantMap fam=o.toVariantMap();
            if(!note.isEmpty()) fam["zuordnungshinweis"]=note;
            r["family"]=fam;
            evidence.append(QStringLiteral("Vorlage: %1 (%2)%3.").arg(o.value("hersteller").toString(),o.value("vertrauen").toString(),
                note.isEmpty() ? QString() : QStringLiteral(" – ")+note));
            break;
        }
        r["pathTypes"]=families.value("reparaturwegTypen").toObject().toVariantMap();

        // --- Known firmware issues --------------------------------------------
        QVariantList issues;
        for(const auto &e:firmware.value("eintraege").toArray()) {
            const auto o=e.toObject();
            QString pattern;
            for(const auto &p:o.value("modellMuster").toArray())
                if(nmodel.contains(p.toString()) && p.toString().size()>pattern.size()) pattern=p.toString();
            if(pattern.isEmpty() || firmwareRev.isEmpty() || firmwareRev==QStringLiteral("Nicht ermittelt")) continue;
            QVariantMap issue=o.toVariantMap();
            bool affected=false;
            for(const auto &b:o.value("betroffeneFirmware").toArray()) if(b.toString().compare(firmwareRev,Qt::CaseInsensitive)==0) affected=true;
            const auto fixed=o.value("korrigierteFirmware");
            if(!affected && fixed.isObject() && o.value("vergleich").toString()==QStringLiteral("kleiner_als_korrigiert")) {
                const QString target=fixed.toObject().value(pattern).toString();
                const QRegularExpression num(QStringLiteral("^(\\d+)"));
                const auto a=num.match(firmwareRev), b=num.match(target);
                if(a.hasMatch() && b.hasMatch() && a.captured(1).toULongLong()<b.captured(1).toULongLong()) affected=true;
                issue["korrigierteFirmware"]=target;
            }
            if(!affected) continue;
            issue["aktuelleFirmware"]=firmwareRev;
            issues.append(issue);
            evidence.append(QStringLiteral("Bekannter Firmwarefehler: %1 mit Firmware %2 – %3").arg(o.value("hersteller").toString(),firmwareRev,o.value("beschreibung").toString()));
        }
        r["firmwareIssues"]=issues;
        r["evidence"]=evidence;
        return r;
    }

private:
    Catalog() {
        pci=load(QStringLiteral("pci_nvme_ids.json"));
        usbStorage=load(QStringLiteral("usb_storage_bridges.json"));
        usbNvme=load(QStringLiteral("usb_nvme_bridges.json"));
        families=load(QStringLiteral("controller_families.json"));
        firmware=load(QStringLiteral("firmware_issues.json"));
    }
    QJsonObject load(const QString &name) {
        const QString override=QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
            .filePath(QStringLiteral("vorlagen/")+name);
        for(const auto &path:{override,QStringLiteral(":/catalog/")+name}) {
            QFile f(path);
            if(!f.open(QIODevice::ReadOnly)) continue;
            const auto doc=QJsonDocument::fromJson(f.readAll());
            if(doc.isObject()) { sources<<path; return doc.object(); }
        }
        return {};
    }
};

inline QVariantMap identify(const QVariantMap &device) { return Catalog::instance().identify(device); }
inline QVariantMap totals() { return Catalog::instance().totals(); }

} // namespace ControllerCatalog

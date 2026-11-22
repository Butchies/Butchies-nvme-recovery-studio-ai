#pragma once
// Board view: picks a schematic M.2 layout and marks the components the
// findings point to (controller, DRAM cache, flash group, power, connector).
// Flash errors mark ALL flash chips: NVMe does not reveal which chip an
// address lives on (internal mapping of the controller).
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

namespace BoardMap {

inline const QJsonArray &layouts() {
    static const QJsonArray l=[]{
        for(const auto &p:{QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath(QStringLiteral("vorlagen/board_layouts.json")),
                           QStringLiteral(":/catalog/board_layouts.json")}) {
            QFile f(p); if(f.open(QIODevice::ReadOnly)) return QJsonDocument::fromJson(f.readAll()).object().value("layouts").toArray();
        }
        return QJsonArray();
    }();
    return l;
}

inline QVariantList layoutList() {
    QVariantList v; for(const auto &l:layouts()) v.append(QVariantMap{{"id",l.toObject().value("id")},{"name",l.toObject().value("name")}}); return v;
}

// User choice wins; otherwise DRAM-less (HMB request) and capacity decide.
inline QString chooseLayout(const QVariantMap &d, const QVariantMap &report, const QString &userChoice) {
    if(!userChoice.isEmpty()) return userChoice;
    const auto eq=report.value("cacheTest").toMap().value("ausstattung").toMap();
    const QString cls=report.value("catalog").toMap().value("controllerName").toString();
    const bool dramless=eq.contains("dramlos") ? eq.value("dramlos").toBool()
        : (cls.contains(QStringLiteral("DRAM-less"),Qt::CaseInsensitive) || cls.contains(QStringLiteral("XT")) || cls.contains(QStringLiteral("DL")));
    const double tb=d.value("sizeBytes").toString().toDouble()/1e12;
    if(dramless) return tb>1.2 ? QStringLiteral("2280_dramlos_2nand") : QStringLiteral("2280_dramlos_1nand");
    return tb>2.5 ? QStringLiteral("2280_dram_4nand") : QStringLiteral("2280_dram_2nand");
}

// Which component do the findings point to?
inline QVariantList faults(const QVariantMap &d, const QVariantMap &report, const QVariantMap &repair) {
    QVariantList out;
    const auto add=[&](const QString &type,const QString &why,const QString &level){
        for(const auto &f:out) if(f.toMap().value("typ")==type && f.toMap().value("grund")==why) return;
        out.append(QVariantMap{{"typ",type},{"grund",why},{"schwere",level}});
    };
    const QStringList feats=report.value("controllerMeldungen").toMap().value("merkmale").toStringList();
    const QString cause=report.value("kiModell").toMap().value("ursache").toString();
    const unsigned warn=d.value("criticalWarning").toString().toUInt(nullptr,16);
    if(d.value("romModeSuspect").toBool()) add(QStringLiteral("controller"),QStringLiteral("Controller im Notmodus / ohne NVMe-Funktion"),QStringLiteral("kritisch"));
    if(feats.contains(QStringLiteral("controller_absturz")) || feats.contains(QStringLiteral("nicht_bereit")) || feats.contains(QStringLiteral("identify_fehler")))
        add(QStringLiteral("controller"),QStringLiteral("Controller stürzt ab bzw. wird nicht bereit"),QStringLiteral("kritisch"));
    if(report.value("cacheTest").toMap().value("selbsttestFehlgeschlagen").toBool())
        add(QStringLiteral("controller"),QStringLiteral("Controller-Selbsttest fehlgeschlagen"),QStringLiteral("kritisch"));
    if(report.value("cacheTest").toMap().value("cacheNichtWirksam").toBool() || cause==QStringLiteral("cache_defekt"))
        add(QStringLiteral("dram"),QStringLiteral("Cache nicht wirksam / nicht angesprochen"),QStringLiteral("hoch"));
    if(report.value("cacheTest").toMap().value("latenzInstabil").toBool())
        add(QStringLiteral("dram"),QStringLiteral("Stark schwankende Antwortzeiten (Cache/Controller)"),QStringLiteral("mittel"));
    if(warn&0x10) add(QStringLiteral("dram"),QStringLiteral("SMART: Sicherungsspeicher/Cache-Schutz ausgefallen"),QStringLiteral("kritisch"));
    if(d.value("mediaErrors").toString().toDouble()>0 || (warn&0x05) || feats.contains(QStringLiteral("io_fehler")) || cause==QStringLiteral("flash_verschleiss"))
        add(QStringLiteral("nand"),QStringLiteral("Flash-Fehler (Medienfehler/Reserve/Lesefehler) – betroffener Chip nicht per NVMe bestimmbar"),QStringLiteral("hoch"));
    const auto z=repair.value("nullfuelltest").toMap();
    if(!z.isEmpty() && !z.value("ok").toBool())
        add(QStringLiteral("nand"),QStringLiteral("Nullfüll-Test: %1 fehlerhafte Blöcke, %2 Lesefehler").arg(z.value("fehlerhafteBloecke").toString(),z.value("lesefehler").toString()),QStringLiteral("kritisch"));
    if(warn&0x08) add(QStringLiteral("nand"),QStringLiteral("SSD im Nur-Lese-Modus (Flash-Reserve erschöpft)"),QStringLiteral("kritisch"));
    if(feats.contains(QStringLiteral("pcie_weg")) || feats.contains(QStringLiteral("apst")))
        add(QStringLiteral("pmic"),QStringLiteral("SSD verschwindet vom PCIe-Bus – Stromversorgung/Energiesparmodus prüfen"),QStringLiteral("hoch"));
    if(warn&0x02) add(QStringLiteral("pmic"),QStringLiteral("Temperatur außerhalb des zulässigen Bereichs"),QStringLiteral("mittel"));
    if(feats.contains(QStringLiteral("pcie_fehler")) || feats.contains(QStringLiteral("pcie_korrigiert")))
        add(QStringLiteral("stecker"),QStringLiteral("PCIe-Übertragungsfehler – Kontakte/Steckplatz prüfen"),QStringLiteral("mittel"));
    if(cause==QStringLiteral("bridge_problem")) add(QStringLiteral("stecker"),QStringLiteral("Verbindung/USB-Gehäuse auffällig (nicht die SSD selbst)"),QStringLiteral("mittel"));
    return out;
}

inline QVariantMap build(const QVariantMap &d, const QVariantMap &report, const QVariantMap &repair, const QString &userChoice) {
    const QString id=chooseLayout(d,report,userChoice);
    QJsonObject layout;
    for(const auto &l:layouts()) if(l.toObject().value("id").toString()==id) layout=l.toObject();
    if(layout.isEmpty() && !layouts().isEmpty()) layout=layouts().first().toObject();
    const auto f=faults(d,report,repair);
    QVariantList parts;
    bool bottom=false;
    for(const auto &p:layout.value("teile").toArray()) {
        QVariantMap m=p.toObject().toVariantMap();
        if(m.value("seite").toString()==QStringLiteral("unten")) bottom=true;
        QStringList why; QString level;
        for(const auto &x:f) if(x.toMap().value("typ")==m.value("typ")) { why<<x.toMap().value("grund").toString(); if(level.isEmpty() || x.toMap().value("schwere")==QStringLiteral("kritisch")) level=x.toMap().value("schwere").toString(); }
        m["defekt"]=!why.isEmpty(); m["gruende"]=why; m["schwere"]=level;
        parts.append(m);
    }
    bool hasDram=false; for(const auto &p:parts) if(p.toMap().value("typ")==QStringLiteral("dram")) hasDram=true;
    QVariantList shown=f;
    if(!hasDram) for(int i=0;i<shown.size();++i) if(shown[i].toMap().value("typ")==QStringLiteral("dram")) {
        auto m=shown[i].toMap(); m["grund"]=m.value("grund").toString()+QStringLiteral(" (DRAM-lose SSD: betrifft Controller/Host-Speicher)"); shown[i]=m;
        bool marked=false;
        for(int k=0;k<parts.size() && !marked;++k) if(parts[k].toMap().value("typ")==QStringLiteral("controller")) { auto c=parts[k].toMap(); c["defekt"]=true; auto g=c.value("gruende").toStringList(); g<<m.value("grund").toString(); c["gruende"]=g; if(c.value("schwere").toString().isEmpty()) c["schwere"]=m.value("schwere"); parts[k]=c; marked=true; }
    }
    return {{"layoutId",layout.value("id").toString()},{"layoutName",layout.value("name").toString()},{"laenge",layout.value("laenge").toDouble()},
            {"breite",layout.value("breite").toDouble()},{"teile",parts},{"beidseitig",bottom},{"befunde",shown},
            {"automatisch",userChoice.isEmpty()}};
}

} // namespace BoardMap

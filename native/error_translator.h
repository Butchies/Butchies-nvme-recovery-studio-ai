#pragma once
// "What does the controller say?" – collects everything the SSD and the
// kernel report and translates it into plain German:
//  - NVMe status codes (error log entries, failed commands)
//  - CSTS controller status, SMART warning bits, self-test results
//  - Linux kernel messages about the device (journalctl/dmesg)
// Tables: data/catalog/nvme_codes.json
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
#include <QProcess>
#include <QRegularExpression>

namespace ErrorTranslator {

inline const QJsonObject &codes() {
    static const QJsonObject c=[]{
        for(const auto &p:{QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath(QStringLiteral("vorlagen/nvme_codes.json")),
                           QStringLiteral(":/catalog/nvme_codes.json")}) {
            QFile f(p); if(f.open(QIODevice::ReadOnly)) return QJsonDocument::fromJson(f.readAll()).object();
        }
        return QJsonObject();
    }();
    return c;
}

// statusField: 15-bit NVMe status field (phase bit removed) or the raw value
// as printed by Linux (e.g. 0x281 / 0x4281 with DNR).
inline QVariantMap status(unsigned value) {
    const unsigned sc=value&0xff, sct=(value>>8)&0x7;
    const bool dnr=(value&0x4000)!=0, more=(value&0x2000)!=0;
    const QString key=QStringLiteral("%1%2").arg(sct).arg(sc,2,16,QLatin1Char('0'));
    const auto e=codes().value("status").toObject().value(key).toObject();
    QVariantMap r;
    r["roh"]=QStringLiteral("Status 0x%1 (Typ %2, Code 0x%3)").arg(value&0x7ff,3,16,QLatin1Char('0')).arg(sct).arg(sc,2,16,QLatin1Char('0'));
    r["typ"]=codes().value("statusTypen").toObject().value(QString::number(sct)).toString();
    if(!e.isEmpty()) {
        r["uebersetzung"]=e.value("name").toString();
        r["bedeutung"]=e.value("bedeutung").toString();
        r["schwere"]=e.value("schwere").toString();
    } else if(sct==7) {
        r["uebersetzung"]=QStringLiteral("Herstellerspezifischer Fehler");
        r["bedeutung"]=QStringLiteral("Nur der Hersteller kann diesen Code genau deuten. Er zeigt, dass der Controller intern ein Problem meldet.");
        r["schwere"]=QStringLiteral("hoch");
    } else {
        r["uebersetzung"]=QStringLiteral("Unbekannter Code");
        r["bedeutung"]=QStringLiteral("Nicht in den Vorlagen verzeichnet.");
        r["schwere"]=QStringLiteral("mittel");
    }
    if(dnr) r["bedeutung"]=r.value("bedeutung").toString()+QStringLiteral(" Der Controller sagt: Wiederholen zwecklos.");
    if(more) r["bedeutung"]=r.value("bedeutung").toString()+QStringLiteral(" Weitere Details im Fehlerprotokoll.");
    return r;
}

inline QVariantMap csts(quint32 v) {
    const auto c=codes().value("csts").toObject();
    const QString hex=QStringLiteral("0x%1").arg(v,0,16);
    QVariantMap r; r["roh"]=QStringLiteral("CSTS=%1").arg(hex);
    const QString special=c.value("sonderwerte").toObject().value(hex).toString();
    QStringList bits;
    if(v!=0xffffffffu) {
        if(v&0x1) bits<<QStringLiteral("bereit");
        if(v&0x2) bits<<QStringLiteral("SCHWERER CONTROLLERFEHLER");
        if((v>>2)&0x3) bits<<QStringLiteral("Herunterfahren aktiv/abgeschlossen");
        if(v&0x10) bits<<QStringLiteral("Subsystem-Reset");
        if(v&0x20) bits<<QStringLiteral("Verarbeitung angehalten");
        if(!(v&0x1)) bits<<QStringLiteral("nicht bereit");
    }
    r["uebersetzung"]=special.isEmpty() ? bits.join(QStringLiteral(", ")) : special;
    r["schwere"]=(v==0xffffffffu || (v&0x2) || !(v&0x1)) ? QStringLiteral("kritisch") : QStringLiteral("info");
    return r;
}

inline QVariantList smartWarnings(unsigned bits) {
    QVariantList l;
    const auto t=codes().value("smartWarnbits").toObject();
    for(int i=0;i<6;++i) if(bits&(1u<<i))
        l.append(QVariantMap{{"roh",QStringLiteral("SMART-Warnbit %1").arg(i)},{"uebersetzung",t.value(QString::number(i)).toString()},{"schwere","kritisch"}});
    return l;
}

// Kernel messages about one controller (Linux), translated.
inline QVariantList kernelMessages(const QString &ctrlName, const QString &pciAddress) {
    QVariantList out;
#ifndef Q_OS_WIN
    QProcess p;
    p.start(QStringLiteral("journalctl"),{QStringLiteral("-k"),QStringLiteral("-b"),QStringLiteral("--no-pager"),QStringLiteral("-o"),QStringLiteral("short-monotonic")});
    if(!p.waitForFinished(10000) || p.exitCode()!=0) { p.start(QStringLiteral("dmesg"),{}); p.waitForFinished(10000); }
    const auto lines=QString::fromUtf8(p.readAllStandardOutput()).split(QLatin1Char('\n'));
    const auto patterns=codes().value("kernelMuster").toArray();
    const QRegularExpression cstsRe(QStringLiteral("CSTS=(0x[0-9a-fA-F]+)"));
    const QRegularExpression statusRe(QStringLiteral("status[: =]+(0x[0-9a-fA-F]+)"),QRegularExpression::CaseInsensitiveOption);
    for(const auto &line:lines) {
        const bool mine=(!ctrlName.isEmpty() && (line.contains(ctrlName+QLatin1Char(':')) || line.contains(ctrlName+QLatin1Char('n'))))
                     || (!pciAddress.isEmpty() && line.contains(pciAddress));
        if(!mine) continue;
        for(const auto &pt:patterns) {
            const auto o=pt.toObject();
            if(!line.contains(QRegularExpression(o.value("muster").toString()))) continue;
            QVariantMap m{{"roh",line.trimmed()},{"uebersetzung",o.value("uebersetzung").toString()},
                          {"bedeutung",o.value("bedeutung").toString()},{"schwere",o.value("schwere").toString()},
                          {"merkmal",o.value("merkmal").toString()},{"quelle","Kernel"}};
            const auto c=cstsRe.match(line);
            if(c.hasMatch()) m["detail"]=csts(c.captured(1).toUInt(nullptr,16)).value("uebersetzung");
            const auto s=statusRe.match(line);
            if(s.hasMatch()) m["detail"]=status(s.captured(1).toUInt(nullptr,16)).value("uebersetzung");
            out.append(m);
            break;
        }
        if(out.size()>=100) break;
    }
#else
    Q_UNUSED(ctrlName) Q_UNUSED(pciAddress)
#endif
    return out;
}

// Everything the device reported, translated (device map from inspect()).
inline QVariantMap report(const QVariantMap &d) {
    QVariantList items;
    if(d.value("smartAvailable").toBool())
        for(const auto &w:smartWarnings(d.value("criticalWarning").toString().toUInt(nullptr,16))) items.append(w);
    for(const auto &e:d.value("recentNvmeErrors").toList()) {
        const auto m=e.toMap();
        const unsigned raw=m.value("statusField").toString().toUInt(nullptr,16);
        auto t=status(m.value("statusOhnePhase").toBool() ? raw : raw>>1);
        t["roh"]=QStringLiteral("Fehlerprotokoll #%1: %2, Queue %3, Befehl %4%5").arg(m.value("errorCount").toString(),t.value("roh").toString())
            .arg(m.value("submissionQueueId").toInt()).arg(m.value("commandId").toInt())
            .arg(m.contains("lba") ? QStringLiteral(", LBA %1").arg(m.value("lba").toString()) : QString());
        t["quelle"]=QStringLiteral("Controller-Fehlerprotokoll");
        items.append(t);
    }
    for(const auto &k:kernelMessages(d.value("controllerPath").toString().section(QLatin1Char('/'),-1),d.value("pciAddress").toString())) items.append(k);
    int critical=0; QStringList features;
    for(const auto &i:items) {
        const auto m=i.toMap();
        if(m.value("schwere").toString()==QStringLiteral("kritisch")) ++critical;
        if(m.contains("merkmal") && !features.contains(m.value("merkmal").toString())) features<<m.value("merkmal").toString();
    }
    QString summary;
    if(items.isEmpty()) summary=QStringLiteral("Der Controller meldet keine Fehler.");
    else summary=QStringLiteral("%1 Meldungen, davon %2 kritisch.").arg(items.size()).arg(critical);
    return {{"eintraege",items},{"kritisch",critical},{"zusammenfassung",summary},{"merkmale",features}};
}

} // namespace ErrorTranslator

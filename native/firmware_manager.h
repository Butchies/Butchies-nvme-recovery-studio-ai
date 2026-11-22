#pragma once
// Firmware manager: standard NVMe Firmware Image Download + Commit.
//
// prepare(): checks every precondition and lists the risks. Nothing is
//            written. The result names an approval code (last four
//            characters of the serial number) the user must type.
// execute(): only with the matching code; flashes, resets if required and
//            verifies that the controller answers with the new revision.
// Windows edition: external USB only, so flashing is refused there and the
// Linux Boot Edition is recommended.
#include <QString>
#include <QVariantMap>
#include <QVariantList>
#include <QFile>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QThread>
#include <functional>
#include "linux_nvme.h"
#include "controller_catalog.h"

namespace FirmwareManager {

constexpr qint64 MaxImageBytes=64LL*1024*1024;

inline QVariantMap prepare(const QVariantMap &device, const QVariantMap &report, const QString &file,
                           const QVariantMap &experience, bool userConfirmsNoBackupNeeded, bool expert=false) {
    QVariantMap plan;
    QVariantList checks;
    bool allowed=true;
    const auto check=[&](const QString &name,bool ok,const QString &detail,bool mandatory=true) {
        checks.append(QVariantMap{{"name",name},{"ok",ok},{"detail",detail},{"pflicht",mandatory}});
        if(mandatory && !ok) allowed=false;
    };
    const bool linuxEdition=device.value("edition").toString()==QStringLiteral("linux");
    check(QStringLiteral("Direkter NVMe-Zugriff"),linuxEdition && !device.value("controllerPath").toString().isEmpty(),
          linuxEdition ? device.value("controllerPath").toString()
                       : QStringLiteral("Windows-Edition arbeitet nur über USB-Bridges, die keine Firmware-Befehle durchreichen. Bitte Linux-Boot-Stick verwenden."));
    check(QStringLiteral("Controller antwortet"),device.value("nvmeIdentifyAvailable").toBool(),
          QStringLiteral("%1, Firmware %2").arg(device.value("nvmeControllerModel").toString(),device.value("nvmeFirmware").toString()));
    check(QStringLiteral("Firmware-Download unterstützt"),device.value("firmwareDownloadSupported").toBool(),
          QStringLiteral("NVMe-OACS Bit 2"));
    check(QStringLiteral("Kein geschütztes Laufwerk"),!device.value("protected").toBool(),device.value("protectedReason").toString());
    const QStringList mounted=device.value("mounted").toStringList();
    check(QStringLiteral("Keine eingebundenen Partitionen"),mounted.isEmpty(),mounted.isEmpty() ? QStringLiteral("frei") : mounted.join(QStringLiteral(", ")));

    QFile f(file);
    QByteArray image;
    if(f.open(QIODevice::ReadOnly) && f.size()<=MaxImageBytes) image=f.readAll();
    const bool sizeOk=!image.isEmpty() && image.size()%4==0;
    check(QStringLiteral("Firmwaredatei gültig"),sizeOk,QStringLiteral("%1, %2 Byte").arg(QFileInfo(file).fileName()).arg(image.size()));
    const QString sha=QString::fromLatin1(QCryptographicHash::hash(image,QCryptographicHash::Sha256).toHex());
    plan["sha256"]=sha;

    // Template knowledge: is there an official target version for this model?
    QString expected;
    for(const auto &i:report.value("firmwareIssues").toList()) expected=i.toMap().value("korrigierteFirmware").toString();
    if(!expected.isEmpty()) {
        const bool nameHint=QFileInfo(file).fileName().contains(expected,Qt::CaseInsensitive);
        check(QStringLiteral("Zielversion laut Vorlage"),nameHint,
              nameHint ? QStringLiteral("Dateiname enthält %1").arg(expected)
                       : QStringLiteral("Vorlage erwartet %1; Dateiname enthält diese Version nicht – bitte Herkunft prüfen").arg(expected),false);
    }
    check(QStringLiteral("Herkunft"),true,QStringLiteral("Nur offizielle Herstellerdatei für exakt dieses Modell verwenden. Das Programm kann die interne Signatur nicht prüfen; der Controller lehnt fremde Images in der Regel ab."),false);

    const auto last=experience.value("device").toMap().value("letzteRettung").toMap();
    const bool backup=last.value("status").toString()==QStringLiteral("abgeschlossen");
    check(QStringLiteral("Datensicherung"),backup || userConfirmsNoBackupNeeded || expert,
          backup ? QStringLiteral("Rettungsabbild vom %1 (%2 %)").arg(last.value("zeit").toString()).arg(last.value("prozent").toDouble(),0,'f',2)
                 : (userConfirmsNoBackupNeeded ? QStringLiteral("Vom Benutzer bestätigt: Daten werden nicht benötigt")
                                               : (expert ? QStringLiteral("Expertenmodus: keine Sicherung verlangt – Daten können verloren gehen")
                                                         : QStringLiteral("Zuerst Rettungsabbild erstellen oder bestätigen, dass die Daten nicht benötigt werden"))),!expert);
    if(device.value("severity").toString()==QStringLiteral("Kritisch") || report.value("severity").toString()==QStringLiteral("Kritisch"))
        check(QStringLiteral("Zustand"),true,QStringLiteral("SSD ist kritisch: Ein Firmware-Update repariert keinen Flash-Verschleiß und kann einen geschwächten Controller endgültig ausfallen lassen."),false);

    const bool slot1ro=device.value("firmwareSlot1ReadOnly").toBool();
    const int slots=device.value("firmwareSlots").toInt();
    const int slot=(slot1ro && slots>1) ? 2 : 0; // 0 = controller chooses
    const bool immediate=device.value("firmwareActivateWithoutReset").toBool();
    plan["slot"]=slot;
    plan["action"]=immediate ? 3 : 1;
    plan["granularity"]=[&]{ const unsigned g=device.value("fwug").toUInt(); return (g==0 || g==0xff) ? 4096u : g*4096u; }();
    plan["oldFirmware"]=device.value("nvmeFirmware").toString();
    plan["slotRevisions"]=device.value("firmwareSlotRevisions");
    plan["file"]=file;
    plan["controllerPath"]=device.value("controllerPath");
    const QString serial=device.value("nvmeControllerSerial").toString();
    plan["approvalCode"]=expert ? QString() : serial.right(4).toUpper();
    plan["expert"]=expert;
    plan["checks"]=checks;
    plan["allowed"]=allowed;
    plan["risks"]=QVariantList{
        QStringLiteral("Stromausfall oder Trennung während der Übertragung kann den Controller unbrauchbar machen."),
        QStringLiteral("Falsche Firmware (anderes Modell/Board/NAND) kann die SSD dauerhaft unbrauchbar machen."),
        QStringLiteral("Nutzerdaten bleiben bei einem Standard-Update normalerweise erhalten – eine Garantie gibt es nicht."),
        immediate ? QStringLiteral("Aktivierung sofort ohne Neustart (vom Controller unterstützt).")
                  : QStringLiteral("Aktivierung durch Controller-Reset direkt nach der Übertragung.")};
    plan["summary"]=allowed
        ? (expert ? QStringLiteral("Expertenmodus: Alle technischen Prüfungen bestanden – Flashen ohne Bestätigungscode möglich.")
                  : QStringLiteral("Alle Pflichtprüfungen bestanden. Zum Flashen den Bestätigungscode eingeben (letzte 4 Zeichen der Seriennummer)."))
        : QStringLiteral("Flashen gesperrt: mindestens eine Pflichtprüfung ist nicht erfüllt.");
    return plan;
}

// Runs in a worker thread. progress(percent, text).
inline QVariantMap execute(const QVariantMap &plan, const QVariantMap &device, const QString &typedCode,
                           const std::function<void(int,const QString&)> &progress) {
    QVariantMap r;
    r["ok"]=false;
    if(!plan.value("allowed").toBool()) { r["message"]=QStringLiteral("Prüfungen nicht bestanden."); return r; }
    if(!plan.value("expert").toBool() && (typedCode.trimmed().toUpper()!=plan.value("approvalCode").toString() || plan.value("approvalCode").toString().isEmpty())) {
        r["message"]=QStringLiteral("Bestätigungscode stimmt nicht – nichts wurde geschrieben."); return r;
    }
#ifdef Q_OS_WIN
    Q_UNUSED(device) Q_UNUSED(progress)
    r["message"]=QStringLiteral("Firmware-Flash ist nur in der Linux-Boot-Edition verfügbar.");
    return r;
#else
    QFile f(plan.value("file").toString());
    if(!f.open(QIODevice::ReadOnly)) { r["message"]=QStringLiteral("Firmwaredatei nicht lesbar."); return r; }
    const QByteArray image=f.readAll();
    if(QString::fromLatin1(QCryptographicHash::hash(image,QCryptographicHash::Sha256).toHex())!=plan.value("sha256").toString()) {
        r["message"]=QStringLiteral("Firmwaredatei hat sich seit der Prüfung verändert – abgebrochen."); return r;
    }
    progress(0,QStringLiteral("Übertrage Firmware …"));
    const QString err=LinuxNvme::flashFirmware(plan.value("controllerPath").toString(),image,plan.value("slot").toInt(),
        plan.value("action").toInt(),plan.value("granularity").toUInt(),[&](int p){ progress(p,QStringLiteral("Übertrage Firmware …")); });
    if(!err.isEmpty()) { r["message"]=err; return r; }
    progress(100,QStringLiteral("Aktiviere und prüfe …"));
    // Verification: controller must answer again and report the revision.
    QVariantMap after;
    for(int i=0;i<30;++i) {
        QThread::sleep(2);
        after=LinuxNvme::inspect(device);
        if(after.value("nvmeIdentifyAvailable").toBool()) break;
    }
    r["newFirmware"]=after.value("nvmeFirmware").toString();
    r["oldFirmware"]=plan.value("oldFirmware").toString();
    r["smartAfter"]=after.value("smartAvailable").toBool();
    r["criticalWarningAfter"]=after.value("criticalWarning").toString();
    if(!after.value("nvmeIdentifyAvailable").toBool()) {
        r["message"]=QStringLiteral("Firmware übertragen, aber der Controller antwortet nicht mehr. Rechner stromlos neu starten und erneut prüfen.");
        return r;
    }
    const bool changed=r.value("newFirmware").toString()!=r.value("oldFirmware").toString();
    r["ok"]=changed;
    r["message"]=changed
        ? QStringLiteral("Firmware aktualisiert: %1 → %2. Controller antwortet, SMART %3.")
              .arg(r.value("oldFirmware").toString(),r.value("newFirmware").toString(),r.value("smartAfter").toBool() ? QStringLiteral("lesbar") : QStringLiteral("nicht lesbar"))
        : QStringLiteral("Controller antwortet, meldet aber weiterhin %1. Aktivierung erfolgt eventuell erst nach einem Kaltstart.").arg(r.value("oldFirmware").toString());
    return r;
#endif
}

} // namespace FirmwareManager

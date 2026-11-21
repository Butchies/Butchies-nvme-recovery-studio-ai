#pragma once
#include <QVariantMap>
#include <QVariantList>
#include <QString>
#include <QStringList>

// Deterministic local decision engine. This is the autonomous diagnostic
// baseline, not an LLM and not a firmware-flashing implementation.
namespace RecoveryAgent {
inline QVariantMap analyze(const QVariantMap &device) {
    QVariantMap result;
    QVariantList evidence;
    QVariantList actions;
    QStringList warnings;
    QString diagnosis;
    QString confidence="Niedrig";
    QString severity="Unbekannt";
    bool canRead=false;

    if(!device.value("readOnly").toBool() ||
       device.value("connection").toString()!="USB" ||
       device.value("storageBus").toString()!="USB") {
        result["diagnosis"]="Gerät gesperrt: externe USB-Herkunft nicht bestätigt.";
        result["severity"]="Gesperrt";
        result["confidence"]="Hoch";
        result["actions"]=actions;
        result["evidence"]=evidence;
        result["automaticAction"]="Keine Aktion";
        return result;
    }
    evidence.append("Windows bestätigt USB-Speicherbus; nur lesende Analyse.");
    canRead=true;
    if(!device.value("nvmeIdentifyAvailable").toBool()) {
        diagnosis="NVMe-Controller über die USB-Bridge nicht identifizierbar.";
        severity="Diagnose eingeschränkt";
        confidence="Hoch (Zugriffsproblem), niedrig (Fehlerursache)";
        warnings << "Kein Nachweis, dass die Bridge NVMe-Kommandos durchreicht.";
        actions.append("USB-Bridge und NVMe-Passthrough-Kompatibilität prüfen.");
        actions.append("Keine Firmwareentscheidung ohne verifizierte Controlleridentität.");
    } else {
        evidence.append(QStringLiteral("NVMe Identify: %1, Firmware %2.")
            .arg(device.value("nvmeControllerModel").toString(),
                 device.value("nvmeFirmware").toString()));
        diagnosis="NVMe-Controller antwortet auf Identify.";
        severity="Noch nicht bestimmt";
        confidence="Mittel";
        actions.append("Controlleridentität und Firmwarestand für spätere Kompatibilitätsprüfung sichern.");
        if(device.value("smartAvailable").toBool()) {
            const auto warning=device.value("criticalWarning").toString();
            const auto wear=device.value("percentageUsed").toString().toUInt();
            const auto spare=device.value("availableSparePercent").toString().toUInt();
            const auto media=device.value("mediaErrors").toString();
            evidence.append(QStringLiteral("SMART: Warnbits %1, Reserve %2%, Verschleiß %3%, Medienfehler %4.")
                .arg(warning).arg(spare).arg(wear).arg(media));
            const bool critical=(warning!="0x00" && warning!="Nicht ermittelt");
            const bool mediaErrors=(media!="0" && media!="Nicht ermittelt");
            if(critical || spare<10 || wear>=100 || mediaErrors) {
                diagnosis="SMART meldet Anzeichen für einen kritischen Zustand oder Medienfehler.";
                severity="Kritisch";
                confidence="Mittel";
                warnings << "SSD nicht durch Reparaturversuche zusätzlich belasten.";
                actions.append("Vor jeder Firmwaremaßnahme Daten auf ein anderes Laufwerk sichern.");
                actions.append("Schonende, lesende Datensicherung priorisieren; keine Schreibreparatur.");
            } else {
                diagnosis="NVMe antwortet; SMART zeigt in den ausgewerteten Basiswerten keinen offensichtlichen kritischen Zustand.";
                severity="Keine akute SMART-Warnung";
                confidence="Mittel";
                actions.append("Fehlerprotokoll auf sporadische Controllerfehler prüfen.");
            }
        } else {
            warnings << "SMART/Health nicht verfügbar; Zustand nicht zuverlässig beurteilbar.";
            actions.append("SMART-Zugriff über kompatiblen Adapter oder Linux-Boot-Edition nachholen.");
        }
        if(device.value("errorLogAvailable").toBool()) {
            const auto errors=device.value("recentNvmeErrors").toList();
            if(!errors.isEmpty()) {
                evidence.append(QStringLiteral("%1 NVMe-Fehlerprotokolleinträge lesbar.").arg(errors.size()));
                actions.append("NVMe-Statuscodes aus dem Fehlerprotokoll in die Ursachenanalyse einbeziehen.");
                if(severity!="Kritisch") severity="Auffällig";
            }
        }
    }
    result["diagnosis"]=diagnosis;
    result["severity"]=severity;
    result["confidence"]=confidence;
    result["evidence"]=evidence;
    result["actions"]=actions;
    result["warnings"]=warnings;
    result["automaticAction"]=canRead
        ? "Lesende Diagnose abgeschlossen; keine sichere Schreibreparatur freigegeben."
        : "Keine Aktion";
    result["requiresApprovalForFirmware"]=true;
    result["firmwareWriteExecuted"]=false;
    return result;
}
}

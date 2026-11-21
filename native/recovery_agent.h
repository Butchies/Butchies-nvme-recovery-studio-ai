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
    result["warnings"]=warnings;
    result["smartAvailable"]=device.value("smartAvailable").toBool();
    if(canRead)
        actions.prepend("Automatisches Rettungsabbild auf ein separates Ziellaufwerk erstellen (SSD wird nur gelesen).");
    result["actions"]=actions;
    result["canImage"]=canRead;
    result["automaticAction"]=canRead
        ? "Lesende Diagnose abgeschlossen. Nächster automatischer Schritt: Rettungsabbild, sobald ein Zielordner angegeben ist."
        : "Keine Aktion";
    result["requiresApprovalForFirmware"]=true;
    result["firmwareWriteExecuted"]=false;
    return result;
}

// Chooses the rescue strategy from the diagnosis. The rescue only reads the
// source SSD; it writes exclusively to a separate destination. No approval
// for destructive actions is required because none are issued.
inline QVariantMap planRescue(const QVariantMap &diagnosis) {
    QVariantMap plan;
    QVariantList decisions;
    const QString severity=diagnosis.value("severity").toString();
    if(!diagnosis.value("canImage").toBool() || severity=="Gesperrt") {
        plan["allowed"]=false;
        plan["reason"]=QStringLiteral("Rettung gesperrt: Gerät ist nicht als externes USB-Laufwerk bestätigt.");
        return plan;
    }
    plan["allowed"]=true;
    decisions.append(QStringLiteral("Datensicherung vor jeder Reparatur: Die SSD wird ausschließlich gelesen, Ziel ist ein separates Laufwerk."));
    decisions.append(QStringLiteral("Keine Firmware-, Reset-, Format- oder Sanitize-Befehle an die SSD."));
    if(severity=="Kritisch") {
        plan["chunkBytes"]=512*1024;
        plan["skipStartBytes"]=4*1024*1024;
        plan["skipMaxBytes"]=1024ULL*1024*1024;
        plan["readTimeoutMs"]=15000;
        plan["retryPasses"]=1;
        plan["maxConsecutiveTimeouts"]=2;
        decisions.append(QStringLiteral("SMART meldet kritischen Zustand oder Medienfehler: schonender Modus mit kleinen Blöcken, großen Sprüngen über Fehlerzonen und kurzen Zeitlimits, damit zuerst möglichst viele gesunde Daten gesichert werden."));
        decisions.append(QStringLiteral("Nur ein Wiederholungsdurchgang für defekte Sektoren, um die SSD nicht zusätzlich zu belasten."));
        decisions.append(QStringLiteral("Bei zwei Zeitüberschreitungen in Folge wird pausiert; der Fortschritt bleibt gespeichert."));
    } else if(severity=="Diagnose eingeschränkt" || !diagnosis.value("smartAvailable",true).toBool()) {
        plan["chunkBytes"]=1024*1024;
        plan["skipStartBytes"]=2*1024*1024;
        plan["skipMaxBytes"]=512ULL*1024*1024;
        plan["readTimeoutMs"]=20000;
        plan["retryPasses"]=1;
        plan["maxConsecutiveTimeouts"]=3;
        decisions.append(QStringLiteral("Zustand über die USB-Bridge nicht vollständig bestimmbar: vorsichtiger Standardmodus mit Fehlersprüngen."));
    } else if(severity=="Auffällig") {
        plan["chunkBytes"]=1024*1024;
        plan["skipStartBytes"]=2*1024*1024;
        plan["skipMaxBytes"]=512ULL*1024*1024;
        plan["readTimeoutMs"]=20000;
        plan["retryPasses"]=2;
        plan["maxConsecutiveTimeouts"]=3;
        decisions.append(QStringLiteral("NVMe-Fehlerprotokoll enthält Einträge: Standardmodus mit Fehlersprüngen und zwei Wiederholungen."));
    } else {
        plan["chunkBytes"]=1024*1024;
        plan["skipStartBytes"]=1024*1024;
        plan["skipMaxBytes"]=256ULL*1024*1024;
        plan["readTimeoutMs"]=30000;
        plan["retryPasses"]=2;
        plan["maxConsecutiveTimeouts"]=3;
        decisions.append(QStringLiteral("Keine akute SMART-Warnung: vollständiges Abbild mit großen Blöcken."));
    }
    decisions.append(QStringLiteral("Lesekarte wird laufend gespeichert; nach Abbruch oder Trennung setzt die Rettung an derselben Stelle fort."));
    decisions.append(QStringLiteral("Nach Abschluss: Abdeckungsprüfung, SHA-256-Prüfsumme und Bericht."));
    plan["decisions"]=decisions;
    return plan;
}

// Evaluates a finished or interrupted rescue and derives the next steps.
inline QVariantMap evaluateRescue(const QVariantMap &diagnosis, const QVariantMap &result) {
    QVariantMap e;
    QVariantList next;
    const QString state=result.value("state").toString();
    const double percent=result.value("rescuedPercent").toDouble();
    const quint64 bad=result.value("badBytes").toULongLong();
    const bool critical=diagnosis.value("severity").toString()=="Kritisch";
    if(state=="fehler") {
        e["summary"]=QStringLiteral("Rettung nicht gestartet: %1").arg(result.value("message").toString());
        e["rating"]="Nicht gestartet";
        next.append("Angegebenen Grund beheben und Rettung erneut starten.");
    } else if(state=="abgeschlossen" && bad==0 && result.value("imageSizeOk").toBool()) {
        e["summary"]=QStringLiteral("Vollständiges Abbild: alle Sektoren gelesen, Größe geprüft, Prüfsumme erstellt.");
        e["rating"]="Vollständig gerettet";
        next.append("Dateien aus abbild.img wiederherstellen; dafür nur das Abbild verwenden, nicht die Original-SSD.");
        next.append(critical ? "SSD wegen kritischem SMART-Zustand nach der Sicherung nicht weiter verwenden."
                             : "SSD erst nach erfolgreicher Prüfung der geretteten Dateien weiterverwenden oder neu einrichten.");
    } else if(state=="abgeschlossen") {
        e["summary"]=QStringLiteral("Teilweise gerettet: %1 % gesichert, %2 Bereiche dauerhaft nicht lesbar.")
            .arg(percent,0,'f',3).arg(result.value("badRegions").toInt());
        e["rating"]="Teilweise gerettet";
        next.append("Dateien aus abbild.img wiederherstellen; nicht lesbare Bereiche sind im Abbild mit Nullen gefüllt.");
        next.append("Liste der nicht lesbaren Bereiche zeigt, welche Daten betroffen sein können.");
        next.append("Die SSD hat defekte Bereiche und sollte nicht mehr für wichtige Daten genutzt werden.");
    } else {
        e["summary"]=QStringLiteral("Rettung %1 bei %2 %: %3").arg(state).arg(percent,0,'f',2).arg(result.value("message").toString());
        e["rating"]="Unterbrochen";
        next.append("Rettung mit demselben Zielordner erneut starten; bereits gesicherte Bereiche werden nicht erneut gelesen.");
        if(critical) next.append("Vor dem Fortsetzen SSD einige Minuten abkühlen lassen und direkt am Rechner (ohne USB-Hub) anschließen.");
    }
    e["nextSteps"]=next;
    return e;
}
}

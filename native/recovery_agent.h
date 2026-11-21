#pragma once
#include <QVariantMap>
#include <QVariantList>
#include <QString>
#include <QStringList>
#include "controller_catalog.h"

// Deterministic local decision engine (not an LLM). Combines measurements,
// controller templates (ControllerCatalog) and learned experience
// (KnowledgeBase) into a diagnosis, repair paths and a rescue plan.
// It never issues write commands to the SSD itself.
namespace RecoveryAgent {
inline bool number(const QVariant &v, double &out) {
    bool ok=false; out=v.toString().toDouble(&ok); return ok;
}

// Diagnosis from current measurements plus learned experience
// (KnowledgeBase::Store::experience) of earlier cases.
inline QVariantMap analyze(const QVariantMap &device, const QVariantMap &experience=QVariantMap()) {
    QVariantMap result;
    QVariantList evidence;
    QVariantList actions;
    QStringList warnings;
    QString diagnosis;
    QString confidence="Niedrig";
    QString severity="Unbekannt";
    bool canRead=false;

    const bool linuxEdition=device.value("edition").toString()=="linux";
    if(linuxEdition && device.value("protected").toBool()) {
        result["diagnosis"]=QStringLiteral("Geschützt: %1. Dieses Laufwerk wird weder gerettet noch repariert.").arg(device.value("protectedReason").toString());
        result["severity"]="Gesperrt";
        result["confidence"]="Hoch";
        result["actions"]=actions;
        result["evidence"]=evidence;
        result["automaticAction"]="Keine Aktion";
        return result;
    }
    if(!device.value("readOnly").toBool() || (!linuxEdition && (
       device.value("connection").toString()!="USB" ||
       device.value("storageBus").toString()!="USB"))) {
        result["diagnosis"]="Gerät gesperrt: externe USB-Herkunft nicht bestätigt.";
        result["severity"]="Gesperrt";
        result["confidence"]="Hoch";
        result["actions"]=actions;
        result["evidence"]=evidence;
        result["automaticAction"]="Keine Aktion";
        return result;
    }
    evidence.append(linuxEdition ? QStringLiteral("Direkter PCIe-Zugriff (Linux-Boot-Edition) auf %1, Treiber %2; nur lesende Analyse.")
                                       .arg(device.value("pciAddress").toString(),device.value("driver").toString())
                                 : QStringLiteral("Windows bestätigt USB-Speicherbus; nur lesende Analyse."));
    canRead=true;
    const QString sizeText=device.value("sizeBytes").toString().trimmed();
    const bool noMedium=sizeText.isEmpty() || sizeText=="0";
    const bool romMode=linuxEdition && device.value("romModeSuspect").toBool();
    if(romMode) {
        diagnosis=QStringLiteral("Notmodus-Verdacht: Controller %1:%2 ist am PCIe-Bus sichtbar (Klasse %3, Treiber %4, Zustand %5), stellt aber keine funktionsfähige NVMe-Schnittstelle bereit. Typisch für ROM-/Safe-Mode nach Firmware- oder Flash-Defekt.")
            .arg(device.value("pciVendorId").toString(),device.value("pciDeviceId").toString(),device.value("pciClass").toString(),
                 device.value("driver").toString(),device.value("nvmeState",QStringLiteral("–")).toString());
        severity="Kritisch";
        confidence="Mittel";
        canRead=false;
        warnings << "Kein Datenzugriff möglich; Rettungsabbild nicht möglich, solange der Controller nicht normal startet.";
        actions.append("Kalt-Neustart (Stromlos, 30 s) und erneute Erkennung; manche Controller starten danach wieder normal.");
        actions.append("Notmodus-Reparatur nur mit passendem Herstellerwerkzeug der Controllerfamilie – setzt die SSD zurück (Datenverlust). Bei wichtigen Daten zuerst Labor erwägen.");
    } else if(noMedium) {
        diagnosis=linuxEdition
            ? QStringLiteral("Controller antwortet, meldet aber keinen nutzbaren Speicherbereich (kein Namespace oder Kapazität 0). Typisch für beschädigte Firmware-Konfiguration oder Flash-Verwaltung.")
            : QStringLiteral("SSD meldet sich nicht: Die USB-Bridge ist vorhanden, liefert aber kein Speichermedium. Der SSD-Controller startet nicht normal (Notmodus, Firmware- oder Flash-Defekt, Stromversorgung oder Kontakt).");
        severity="Kritisch";
        confidence="Mittel";
        canRead=false;
        warnings << "Kein Datenzugriff über USB möglich; ein Rettungsabbild kann nicht erstellt werden.";
        actions.append("Hardware prüfen: anderes Gehäuse/Kabel, direkter USB-Anschluss ohne Hub, SSD-Kontakte und Temperatur.");
        if(!linuxEdition) actions.append("SSD direkt in einen M.2-Steckplatz setzen und mit der Linux-Boot-Edition auf Notmodus prüfen (Controller-Identifikation über PCIe).");
    } else if(!device.value("nvmeIdentifyAvailable").toBool()) {
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
    // --- Learned experience ---------------------------------------------------
    int learned=0;
    if(canRead && !experience.isEmpty()) {
        const auto dev=experience.value("device").toMap();
        const auto bridge=experience.value("bridge").toMap();
        const auto ctrl=experience.value("controller").toMap();
        const QString bridgeKey=experience.value("bridgeKey").toString();
        if(dev.value("diagnosen").toInt()>0) {
            ++learned;
            evidence.append(QStringLiteral("Gerät bekannt: %1 frühere Diagnosen, zuletzt %2 (Bewertung: %3).")
                .arg(dev.value("diagnosen").toInt()).arg(dev.value("letzteUntersuchung").toString(),dev.value("letzteBewertung").toString()));
        }
        // Trend: compare with the SMART snapshot of the previous examination.
        const auto prev=dev.value("letzterSmart").toMap();
        if(!prev.isEmpty() && device.value("smartAvailable").toBool()) {
            double a=0,b=0;
            if(number(prev.value("medienfehler"),a) && number(device.value("mediaErrors"),b) && b>a) {
                ++learned;
                warnings << QStringLiteral("Medienfehler seit der letzten Untersuchung gestiegen (%1 → %2): Die SSD verschlechtert sich aktiv.").arg(a,0,'f',0).arg(b,0,'f',0);
                diagnosis+=QStringLiteral(" Verlauf: Medienfehler steigen – fortschreitender Defekt.");
                severity="Kritisch";
                actions.prepend("Sofort sichern: Die SSD verschlechtert sich zwischen den Untersuchungen.");
            }
            if(number(prev.value("reserve"),a) && number(device.value("availableSparePercent"),b) && a-b>=5) {
                ++learned;
                warnings << QStringLiteral("Ersatzblock-Reserve gesunken (%1 % → %2 %).").arg(a,0,'f',0).arg(b,0,'f',0);
                if(severity!="Kritisch") severity="Auffällig";
            }
            if(number(prev.value("fehlerprotokoll"),a) && number(device.value("errorLogEntries"),b) && b>a) {
                ++learned;
                evidence.append(QStringLiteral("Controller-Fehlerprotokoll seit letzter Untersuchung um %1 Einträge gewachsen.").arg(b-a,0,'f',0));
                if(severity!="Kritisch") severity="Auffällig";
            }
        }
        const auto last=dev.value("letzteRettung").toMap();
        if(!last.isEmpty()) {
            ++learned;
            evidence.append(QStringLiteral("Letzte Rettung dieser SSD: %1, %2 % gesichert, %3 Zeitüberschreitungen.")
                .arg(last.value("status").toString()).arg(last.value("prozent").toDouble(),0,'f',2).arg(last.value("zeitueberschreitungen").toInt()));
        }
        const int bCases=bridge.value("faelle").toInt();
        if(bCases>0) {
            ++learned;
            const int idOk=bridge.value("identifyOk").toInt();
            const bool idNow=device.value("nvmeIdentifyAvailable").toBool();
            if(!idNow && idOk==0) {
                evidence.append(QStringLiteral("USB-Bridge %1 hat in allen %2 bisherigen Fällen keine NVMe-Daten durchgereicht: Die fehlende Controllerdiagnose liegt sehr wahrscheinlich an der Bridge, nicht an der SSD.").arg(bridgeKey).arg(bCases));
                confidence="Hoch (Bridge-Einschränkung, aus Erfahrung)";
                actions.append("Für Controllerdiagnose ein Gehäuse mit NVMe-Passthrough oder die Linux-Boot-Edition verwenden.");
            } else if(!idNow && idOk>0) {
                warnings << QStringLiteral("Diese USB-Bridge lieferte in %1 früheren Fällen NVMe-Daten, jetzt nicht: Hinweis auf ein Problem der SSD bzw. ihres Controllers.").arg(idOk);
                if(severity!="Kritisch") severity="Auffällig";
                diagnosis+=QStringLiteral(" Erfahrung: Die Bridge kann NVMe durchreichen – der Controller antwortet vermutlich nicht korrekt.");
            }
            if(bridge.value("verbindungsverluste").toInt()>0)
                warnings << QStringLiteral("Mit dieser USB-Bridge gab es bisher %1 Verbindungsabbrüche während Rettungen.").arg(bridge.value("verbindungsverluste").toInt());
        }
        if(ctrl.value("faelle").toInt()>1) {
            ++learned;
            const int r=ctrl.value("rettungen").toInt();
            evidence.append(QStringLiteral("Controller %1: %2 bekannte Fälle, davon %3 kritisch%4.")
                .arg(experience.value("controllerKey").toString()).arg(ctrl.value("faelle").toInt()).arg(ctrl.value("kritisch").toInt())
                .arg(r>0 ? QStringLiteral("; Ø %1 % gerettet in %2 Rettungen").arg(ctrl.value("prozentSumme").toDouble()/r,0,'f',1).arg(r) : QString()));
        }
    }
    // --- Controller templates ---------------------------------------------
    const QVariantMap cat=ControllerCatalog::identify(device);
    for(const auto &e:cat.value("evidence").toList()) evidence.append(e);
    if(cat.value("bridgeIsSata").toBool())
        warnings << "Das Gehäuse ist laut Vorlagen eine SATA-Bridge: NVMe-SSDs funktionieren darin nicht, nur M.2-SATA-SSDs.";
    const auto issues=cat.value("firmwareIssues").toList();
    for(const auto &i:issues) {
        const auto m=i.toMap();
        warnings << QStringLiteral("Bekannter Firmwarefehler (%1): Firmware %2 → Update auf %3.")
            .arg(m.value("schwere").toString(),m.value("aktuelleFirmware").toString(),m.value("korrigierteFirmware").toString());
        actions.prepend(QStringLiteral("Firmware-Update auf %1 aus offizieller Quelle (%2) – erst nach Datensicherung und nur mit Ihrer Freigabe.")
            .arg(m.value("korrigierteFirmware").toString(),m.value("quelleFirmware").toString()));
        if(m.value("schwere").toString()=="kritisch" && severity!="Kritisch") severity="Auffällig";
    }
    // Repair paths from the family template, rated for the current situation.
    QVariantList paths;
    const auto types=cat.value("pathTypes").toMap();
    const auto family=cat.value("family").toMap();
    QVariantList familyPaths=family.value("reparaturwege").toList();
    if(familyPaths.isEmpty())
        familyPaths={QVariantMap{{"typ","datenrettung"}},QVariantMap{{"typ","nvme_firmware_update"}},QVariantMap{{"typ","labor"}}};
    if(noMedium || romMode) familyPaths.prepend(QVariantMap{{"typ","hardware_pruefung"}});
    for(const auto &fp:familyPaths) {
        auto p=fp.toMap();
        const QString typ=p.value("typ").toString();
        p["beschreibung"]=types.value(typ).toString();
        QString status;
        if(typ=="datenrettung") status=canRead ? "Jetzt automatisch möglich" : "Über diese Verbindung nicht möglich";
        else if(typ=="hardware_pruefung") status="Jetzt durch Sie möglich";
        else if(typ=="nvme_firmware_update") {
            const QString how=linuxEdition
                ? (device.value("firmwareDownloadSupported").toBool() ? QStringLiteral("Firmware-Manager: jetzt möglich (Freigabe nötig)")
                                                                      : QStringLiteral("Controller meldet keine Firmware-Download-Unterstützung"))
                : QStringLiteral("über USB nicht möglich – Linux-Boot-Stick mit SSD im M.2-Steckplatz verwenden");
            status=(issues.isEmpty() ? QStringLiteral("Kein bekannter Firmwarefehler; nur bei Bedarf. ") : QStringLiteral("Empfohlen. "))+how;
        }
        else if(typ=="notmodus_hersteller_werkzeug")
            status=linuxEdition ? (romMode ? "Passt zum Befund – nur mit registriertem Herstellerwerkzeug, Freigabe und Datenverlust"
                                           : "Nur bei Notmodus nötig; Datenverlust")
                                : "Nur mit Linux-Boot-Edition und vom Nutzer bereitgestelltem Herstellerwerkzeug; Datenverlust";
        else if(typ=="labor") status="Immer möglich (extern)";
        p["status"]=status;
        paths.append(p);
    }
    result["catalog"]=cat;
    result["repairPaths"]=paths;
    result["firmwareIssues"]=issues;
    result["learnedFindings"]=learned;
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
inline QVariantMap planRescue(const QVariantMap &diagnosis, const QVariantMap &experience=QVariantMap()) {
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
    // --- Adaptation from experience -------------------------------------------
    int adjustments=0;
    const auto last=experience.value("device").toMap().value("letzteRettung").toMap();
    const auto bridge=experience.value("bridge").toMap();
    const auto ctrl=experience.value("controller").toMap();
    if(last.value("status").toString()=="unterbrochen" && last.value("zeitueberschreitungen").toInt()>0) {
        const qint64 chunk=qMax<qint64>(128*1024,plan.value("chunkBytes").toLongLong()/2);
        const int timeout=qMax(8000,plan.value("readTimeoutMs").toInt()*6/10);
        plan["chunkBytes"]=chunk; plan["readTimeoutMs"]=timeout;
        decisions.append(QStringLiteral("Gelernt: Die letzte Rettung dieser SSD wurde durch Zeitüberschreitungen unterbrochen → kleinere Blöcke (%1 KiB) und kürzeres Zeitlimit (%2 s).").arg(chunk/1024).arg(timeout/1000));
        ++adjustments;
    }
    if(last.value("status").toString()=="abgeschlossen") {
        decisions.append(QStringLiteral("Gelernt: Für diese SSD gibt es bereits eine abgeschlossene Rettung (%1 %). Mit demselben Zielordner werden nur noch defekte Bereiche erneut versucht.").arg(last.value("prozent").toDouble(),0,'f',2));
        ++adjustments;
    }
    if(bridge.value("verbindungsverluste").toInt()>0) {
        plan["reconnectWaitSeconds"]=10;
        decisions.append(QStringLiteral("Gelernt: Diese USB-Bridge hat bei früheren Rettungen %1-mal die Verbindung verloren → längere Wartezeit vor dem Neuverbinden.").arg(bridge.value("verbindungsverluste").toInt()));
        ++adjustments;
    }
    const int rateSamples=bridge.value("rateMessungen").toInt();
    if(severity=="Keine akute SMART-Warnung" && rateSamples>0 && bridge.value("zeitueberschreitungen").toInt()==0
       && bridge.value("verbindungsverluste").toInt()==0) {
        plan["chunkBytes"]=4*1024*1024;
        decisions.append(QStringLiteral("Gelernt: Diese USB-Bridge lief bisher stabil (Ø %1 MiB/s) → größere Blöcke für eine schnellere Sicherung.")
            .arg(bridge.value("rateSumme").toDouble()/rateSamples/1048576.0,0,'f',1));
        ++adjustments;
    }
    if(ctrl.value("unterbrochen").toInt()>=2 && ctrl.value("unterbrochen").toInt()>ctrl.value("vollstaendig").toInt()) {
        plan["maxConsecutiveTimeouts"]=qMin(plan.value("maxConsecutiveTimeouts").toInt(),2);
        plan["retryPasses"]=qMin(plan.value("retryPasses").toInt(),1);
        decisions.append(QStringLiteral("Gelernt: Rettungen mit diesem Controller-Modell brechen häufig ab (%1-mal) → schonendere Fehlerbehandlung.").arg(ctrl.value("unterbrochen").toInt()));
        ++adjustments;
    }
    plan["learnedAdjustments"]=adjustments;
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

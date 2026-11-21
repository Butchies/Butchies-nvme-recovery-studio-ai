"""Offline read-only triage checklist for NVMe recovery cases.

This module provides cautious guidance only. It never sends device commands.
"""
from datetime import datetime, timezone

def plan(report):
    report=report if isinstance(report,dict) else {}
    device=report.get("device") if isinstance(report.get("device"),dict) else {}
    diagnostics=report.get("diagnostics") if isinstance(report.get("diagnostics"),dict) else {}
    usb=report.get("usb_diagnostics") if isinstance(report.get("usb_diagnostics"),dict) else {}
    protection=report.get("safety") if isinstance(report.get("safety"),dict) else {}
    assessment=report.get("assessment") if isinstance(report.get("assessment"),dict) else {}
    steps=[]
    def add(priority,title,reason):
        steps.append({"priority":priority,"title":title,"reason":reason,"automatic_action":False})
    add(1,"Schreibzugriffe unterlassen",
        "Kein Firmware-Flash, kein Formatieren, keine Initialisierung und keine Reparaturbefehle ohne belastbare Diagnose.")
    if not device:
        add(2,"Datenträger eindeutig identifizieren",
            "Modell, Größe, Seriennummer und Anschlussart nur lesend erfassen.")
    if device.get("bus")=="USB":
        add(2,"USB-Bridge-Fähigkeiten klären",
            "USB-Erkennung belegt keine NVMe-Admin-Kommandos. Andere kompatible Bridge nur bei sicherem Hardwareumgang prüfen.")
    if usb.get("error"):
        add(2,"USB-Verbindungsproblem dokumentieren",str(usb["error"])[:500])
    if diagnostics.get("error"):
        add(2,"Diagnosezugriff klären",str(diagnostics["error"])[:500])
    smart=diagnostics.get("smart") or {}
    if isinstance(smart,dict):
        if smart.get("error"):
            add(2,"SMART-Abfrage nicht möglich",str(smart["error"])[:500])
        try: critical=int(str(smart.get("critical_warning",0)),0)
        except (ValueError,TypeError): critical=0
        if critical:
            add(1,"NVMe-Warnbits dokumentieren",
                "Critical-Warning-Bits gesetzt; Schreibzugriffe vermeiden und professionelle Datenrettung erwägen.")
        try: errors=int(str(smart.get("media_errors",0)),0)
        except (ValueError,TypeError): errors=0
        if errors>0:
            add(1,"Medienfehler berücksichtigen",
                "Fehlerzähler ist größer als null. Zusätzliche Belastung vermeiden.")
    if isinstance(diagnostics.get("smartctl"),dict):
        ata=diagnostics["smartctl"]
        for warning in diagnostics.get("warnings",[]) if isinstance(diagnostics.get("warnings"),list) else []:
            if isinstance(warning,str):add(2,"ATA-Diagnosehinweis",warning[:500])
        health=ata.get("smart_status")
        if isinstance(health,dict) and health.get("passed") is False:
            add(1,"ATA-SMART-Warnung ernst nehmen",
                "SMART-Gesamtstatus meldet einen Fehler. Laufwerk nicht zusätzlich belasten.")
        if assessment.get("status") in ("kritisch","auffällig"):
            add(1,"Auffällige SATA/ATA-Messwerte sichern",
                "Vorhandene SMART-Belege dokumentieren; keine automatischen Reparatur- oder Selbsttests starten.")
        if diagnostics.get("command_ok") is False:
            add(2,"ATA-SMART-Daten auf Vollständigkeit prüfen",
                "smartctl meldete einen nicht erfolgreichen Rückgabestatus; vorhandene Teilinformationen sind keine Entwarnung.")
    capabilities=report.get("nvme_capabilities") if isinstance(report.get("nvme_capabilities"),dict) else {}
    if capabilities:
        if capabilities.get("firmware_download_commit_advertised") is False:
            add(2,"Firmware-Kommandos nicht angekündigt",
                "Controller meldet keine Standard-Firmware-Download/Commit-Fähigkeit.")
        if capabilities.get("slot_1_read_only"):
            add(2,"Firmware-Slot 1 schreibgeschützt",
                "Controller meldet Schreibschutz für Slot 1. Keine Umgehungsversuche.")
        limits=capabilities.get("limitations")
        if isinstance(limits,list):
            for limitation in limits:
                if isinstance(limitation,str) and ("außerhalb" in limitation or "Mehr Slot-Einträge" in limitation):
                    add(2,"Widersprüchliche Firmware-Slot-Daten",limitation[:500])
    probe=report.get("read_probe") if isinstance(report.get("read_probe"),dict) else {}
    if probe:
        if probe.get("error") or probe.get("probe_success") is False:
            add(1,"Rohdaten-Leseproblem dokumentieren",
                "Die begrenzte Leseprobe war nicht vollständig erfolgreich. Keine wiederholten Leseversuche erzwingen; zuerst Stromversorgung, Controllerzustand und Datenrettungsrisiko beurteilen.")
        sector=probe.get("sector_evidence") if isinstance(probe.get("sector_evidence"),dict) else {}
        if sector.get("partition_table")=="protective_mbr" and not sector.get("gpt_primary_header_candidates"):
            add(2,"GPT-Header noch nicht nachgewiesen",
                "Ein Schutz-MBR allein beweist keine intakte GPT. Keine Partitionstabelle reparieren oder neu schreiben.")
        candidates=sector.get("gpt_primary_header_candidates")
        if isinstance(candidates,list):
            for candidate in candidates:
                if not isinstance(candidate,dict):
                    continue
                if candidate.get("header_crc_valid") is False:
                    add(1,"GPT-Header-Prüfsumme auffällig",
                        "Der vorhandene GPT-Header-Kandidat hat keine gültige CRC32. Vor jeder Reparatur ein vollständiges Abbild anstreben.")
                if candidate.get("partition_array_crc_valid") is False:
                    add(1,"GPT-Partitionseinträge-Prüfsumme auffällig",
                        "Die vollständig erfasste Partitionseintragstabelle stimmt nicht mit ihrer CRC32 überein.")
                if candidate.get("partition_array_crc_valid") is None:
                    add(3,"GPT-Partitionseinträge unvollständig erfasst",
                        "Die 4-KiB-Leseprobe reicht für eine CRC32-Prüfung der vollständigen GPT-Eintragstabelle nicht aus.")
                if candidate.get("layout_plausible") is False:
                    add(2,"GPT-Layout widersprüchlich",
                        "LBA-Grenzen oder Eintragsgröße sind unplausibel. Keine automatische Korrektur.")
    imaging=report.get("imaging_plan") if isinstance(report.get("imaging_plan"),dict) else {}
    if imaging:
        evidence=imaging.get("destination_evidence")
        if isinstance(evidence,dict) and evidence.get("same_device_detected"):
            add(1,"Quell-/Zielkonflikt",
                "Das gewählte Abbildziel liegt auf dem Quelllaufwerk. Anderes physisches Zielmedium wählen.")
    if protection.get("reasons"):
        add(1,"Systemlaufwerksschutz beachten",
            "Die Schutzprüfung hebt keine Sperren auf; Mounts, Swap und Bootabhängigkeiten können kritisch sein.")
    add(3,"Lesende Datensicherung planen",
        "Vor einer späteren Imaging-Funktion ein separates Zielmedium mit ausreichend Kapazität wählen; Imaging ist noch nicht implementiert.")
    add(3,"Firmware nur als Forschungsbefund behandeln",
        "Hash- und Katalogtreffer sind keine Echtheits- oder Kompatibilitätsbestätigung.")
    return {"generated":datetime.now(timezone.utc).isoformat(),
            "mode":"offline_read_only_guidance",
            "steps":sorted(steps,key=lambda x:x["priority"]),
            "device_writes_allowed":False,
            "warning":"Keine automatische Datenrettung, kein Flash und keine Erfolgsprognose."}

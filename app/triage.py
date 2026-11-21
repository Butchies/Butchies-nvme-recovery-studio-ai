"""Offline read-only triage checklist for NVMe recovery cases.

This module provides cautious guidance only. It never sends device commands.
"""
from datetime import datetime, timezone

def plan(report):
    report=report if isinstance(report,dict) else {}
    device=report.get("device") or {}
    diagnostics=report.get("diagnostics") or {}
    usb=report.get("usb_diagnostics") or {}
    protection=report.get("safety") or {}
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

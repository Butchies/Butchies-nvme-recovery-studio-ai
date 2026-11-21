"""Evidence-based, offline interpretation of available NVMe diagnostics.

No generative model is bundled. This module reports observable conditions and
does not estimate recovery success or authorize device changes.
"""
from datetime import datetime, timezone

def _number(value):
    try:
        if isinstance(value, bool): return None
        if isinstance(value, str): return int(value, 0) if value.startswith("0x") else int(value)
        return int(value)
    except (TypeError, ValueError, OverflowError): return None

def _find(data, *keys):
    if not isinstance(data, dict): return None
    for key in keys:
        if key in data: return data[key]
    return None

def assess(report):
    findings=[]
    diagnostics=report.get("diagnostics", {}) if isinstance(report,dict) else {}
    if not isinstance(diagnostics,dict): diagnostics={}
    if "error" in diagnostics:
        findings.append({"level":"unbekannt","title":"Diagnose nicht verfügbar",
                         "evidence":str(diagnostics["error"]),
                         "advice":"Verbindung, Zugriffsrechte und Edition prüfen. Keine Aussage über SSD-Gesundheit möglich."})
    smart=diagnostics.get("smart",{})
    if isinstance(smart,dict) and not smart.get("error"):
        critical=_number(_find(smart,"critical_warning","critical_warning_raw"))
        if critical is not None:
            findings.append({"level":"warnung" if critical else "info",
                             "title":"NVMe Critical Warning",
                             "evidence":f"critical_warning = {critical}",
                             "advice":"Gesetzte Warnbits anhand der Gerätespezifikation interpretieren; Datenrettung priorisieren." if critical else "Kein Critical-Warning-Bit gesetzt. Andere Fehler sind damit nicht ausgeschlossen."})
        for keys,title,unit in [
            (("media_errors","media_and_data_integrity_errors"),"Medien-/Datenintegritätsfehler",""),
            (("num_err_log_entries","number_of_error_information_log_entries"),"Fehlerlog-Einträge",""),
            (("percentage_used",),"Angezeigter Verschleiß"," %"),
            (("unsafe_shutdowns",),"Unsichere Abschaltungen","")]:
            val=_number(_find(smart,*keys))
            if val is not None:
                findings.append({"level":"warnung" if val>0 and title in ("Medien-/Datenintegritätsfehler","Fehlerlog-Einträge") else "info",
                                 "title":title,"evidence":f"{val}{unit}",
                                 "advice":"Kumulativer Gerätewert; Verlauf und weitere Befunde berücksichtigen."})
    elif "smart" in diagnostics:
        findings.append({"level":"unbekannt","title":"SMART nicht auslesbar",
                         "evidence":str(smart.get("error","Keine verwertbare Antwort")) if isinstance(smart,dict) else "Unbekanntes Format",
                         "advice":"Fehlende SMART-Daten sind kein Beleg für einen Defekt."})
    if not findings:
        findings.append({"level":"unbekannt","title":"Keine ausreichenden NVMe-Messwerte",
                         "evidence":"Kein interpretierbarer SMART-/Diagnosebefund vorhanden.",
                         "advice":"Nur lesende Diagnosedaten erfassen; keine automatische Reparatur."})
    return {"generated":datetime.now(timezone.utc).isoformat(),
            "method":"lokale regelbasierte Auswertung (kein KI-Modell)",
            "findings":findings,"warning":"Keine Erfolgsprognose oder Freigabe für Firmware-Schreibzugriffe."}

def compare(before, after):
    """Compare SMART numeric values only when the same physical device is identifiable."""
    a=before.get("device",{}) if isinstance(before,dict) else {}
    b=after.get("device",{}) if isinstance(after,dict) else {}
    if not isinstance(a,dict) or not isinstance(b,dict):
        return {"error":"Geräteidentität fehlt"}
    serial_a=str(a.get("serial") or "").strip()
    serial_b=str(b.get("serial") or "").strip()
    if not serial_a or not serial_b or serial_a!=serial_b:
        return {"error":"Vergleich gesperrt: Seriennummer fehlt oder unterscheidet sich"}
    if a.get("name")!=b.get("name") or a.get("size")!=b.get("size"):
        return {"error":"Vergleich gesperrt: Modell oder Kapazität unterscheidet sich"}
    sa=(before.get("diagnostics") or {}).get("smart") or {}
    sb=(after.get("diagnostics") or {}).get("smart") or {}
    if not isinstance(sa,dict) or not isinstance(sb,dict):
        return {"error":"SMART-Daten fehlen"}
    changes=[]
    for field in ("media_errors","num_err_log_entries","percentage_used","unsafe_shutdowns","power_on_hours","data_units_read","data_units_written"):
        x,y=_number(sa.get(field)),_number(sb.get(field))
        if x is not None and y is not None:
            changes.append({"field":field,"before":x,"after":y,"difference":y-x})
    return {"device":serial_a,"changes":changes,"note":"Nur identische, auslesbare SMART-Felder verglichen. Kein Defektnachweis."}

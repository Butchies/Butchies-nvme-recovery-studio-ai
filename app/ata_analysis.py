"""Evidence-based, read-only interpretation of ATA SMART JSON."""
def assess(report):
    data=report.get("diagnostics",{}) if isinstance(report,dict) else {}
    payload=data.get("smartctl",{}) if isinstance(data,dict) else {}
    result={"type":"ATA SMART assessment","status":"unbekannt","findings":[],
            "evidence":[],"limitations":["SMART ist keine Garantie gegen plötzlichen Ausfall.",
            "Keine automatischen Reparaturen, Selbsttests oder Schreibzugriffe."]}
    if not isinstance(payload,dict) or not payload:
        result["findings"].append("Keine auswertbaren ATA-SMART-Daten.")
        return result
    health=payload.get("smart_status")
    passed=health.get("passed") if isinstance(health,dict) else None
    if passed is False:
        result["status"]="kritisch"
        result["findings"].append("SMART-Gesamtstatus meldet einen Fehler.")
        result["evidence"].append({"field":"smart_status.passed","value":False})
    elif passed is True:
        result["status"]="SMART-Gesamtstatus bestanden"
        result["findings"].append("SMART-Gesamtstatus bestanden; kein Beweis für ein gesundes Laufwerk.")
        result["evidence"].append({"field":"smart_status.passed","value":True})
    else:
        result["findings"].append("SMART-Gesamtstatus fehlt oder ist nicht interpretierbar.")
    table=payload.get("ata_smart_attributes")
    attrs=table.get("table",[]) if isinstance(table,dict) else []
    flagged=[]
    for attr in attrs if isinstance(attrs,list) else []:
        if not isinstance(attr,dict):continue
        aid=attr.get("id")
        raw=attr.get("raw")
        value=raw.get("value") if isinstance(raw,dict) else None
        if aid in (5,197,198) and isinstance(value,int) and value>0:
            flagged.append({"id":aid,"raw":value})
    if flagged:
        result["status"]="auffällig" if result["status"]!="kritisch" else "kritisch"
        result["findings"].append("Nicht-null SMART-Rohzähler für umverteilte, schwebende oder nicht korrigierbare Sektoren.")
        result["evidence"].append({"field":"ata_smart_attributes.table","attributes":flagged})
    result["limitations"].append("Herstellerabhängige Attribute und SSD-spezifische Zähler sind nicht vollständig bewertet.")
    return result

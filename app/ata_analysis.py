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

def compare(before,after):
    """Compare only same-device ATA raw counters; never infer health from missing data."""
    if not isinstance(before,dict) or not isinstance(after,dict):
        return {"error":"Ungültige Fallakten"}
    a,b=before.get("device"),after.get("device")
    if not isinstance(a,dict) or not isinstance(b,dict):
        return {"error":"Geräteidentität fehlt"}
    for field in ("serial","name","size"):
        if not a.get(field) or a.get(field)!=b.get(field):
            return {"error":"Vergleich gesperrt: Geräteidentität nicht eindeutig gleich","field":field}
    def counters(record):
        diag=record.get("diagnostics")
        if not isinstance(diag,dict):return None
        smart=diag.get("smartctl")
        if not isinstance(smart,dict) or not smart:return None
        attrs=smart.get("ata_smart_attributes")
        table=attrs.get("table") if isinstance(attrs,dict) else None
        if not isinstance(table,list):return None
        found={}
        for entry in table:
            if not isinstance(entry,dict):continue
            key=entry.get("id")
            raw=entry.get("raw")
            value=raw.get("value") if isinstance(raw,dict) else None
            if isinstance(key,int) and not isinstance(value,bool) and isinstance(value,int) and value>=0:
                found[key]=value
        return found
    x,y=counters(before),counters(after)
    if x is None or y is None:return {"error":"ATA-SMART-Attributtabellen fehlen"}
    changes=[{"attribute_id":key,"before":x[key],"after":y[key],"difference":y[key]-x[key]}
             for key in sorted(x.keys() & y.keys())]
    if not changes:return {"error":"Keine gemeinsamen numerischen ATA-SMART-Attribute"}
    return {"type":"ATA SMART comparison","changes":changes,
            "counter_decreases":[item["attribute_id"] for item in changes if item["difference"]<0],
            "warning":"Herstellerabhängige Rohwerte; Zählerrücksetzungen und Firmwareänderungen möglich. Kein Gesundheitsnachweis."}

"""Conservative read-only Linux ATA/SATA SMART diagnostics.

M.2 form factor cannot be proven from ATA transport. No self-tests,
SMART setting changes, or device writes are performed.
"""
import json
import re
import shutil
import sys
from datetime import datetime, timezone
import diagnostics

def report(device):
    if not sys.platform.startswith("linux"):
        return {"error":"ATA-Diagnose ist nur im Linux-Modus verfügbar."}
    if not isinstance(device,dict):
        return {"error":"Ungültige Geräteauswahl."}
    path=str(device.get("path") or "")
    if not re.fullmatch(r"/dev/(?:sd[a-z]+|hd[a-z]+)",path):
        return {"error":"Nur eindeutig erkannte SATA/ATA-Datenträger werden unterstützt."}
    if device.get("bus")!="SATA" and device.get("bus")!="ATA":
        return {"error":"Nur direkt angeschlossene SATA/ATA-Laufwerke. USB-Bridge nicht bestätigt."}
    if not shutil.which("smartctl"):
        return {"error":"smartmontools (smartctl) fehlt."}
    check=diagnostics.run(["lsblk","-J","-b","-d","-o","NAME,SERIAL,MODEL,SIZE,TRAN,TYPE,PATH"])
    try: rows=json.loads(check["stdout"]).get("blockdevices",[]) if check["ok"] else []
    except (ValueError,TypeError,AttributeError): rows=[]
    matches=[x for x in rows if isinstance(x,dict) and x.get("path")==path and x.get("type")=="disk"]
    if len(matches)!=1:
        return {"error":"Datenträger nicht mehr eindeutig vorhanden."}
    current=matches[0]
    try: same_size=int(current.get("size") or 0)==int(device.get("size") or 0)
    except (ValueError,TypeError): same_size=False
    if (not same_size or not str(device.get("serial") or "").strip() or
        str(current.get("serial") or "").strip()!=str(device.get("serial") or "").strip() or
        str(current.get("tran") or "").lower() not in ("sata","ata")):
        return {"error":"Geräteidentität oder Anschluss seit Auswahl verändert."}
    response=diagnostics.run(["smartctl","--json","--info","--health","--attributes","--log=error",path],timeout=20)
    try: payload=json.loads(response["stdout"]) if response["stdout"] else None
    except (ValueError,TypeError): payload=None
    if not isinstance(payload,dict):
        return {"error":response["stderr"] or "Keine auswertbare SMART-JSON-Antwort."}
    return {"device":path,"timestamp":datetime.now(timezone.utc).isoformat(),
            "diagnostic_type":"ATA/SATA SMART (read-only)","smartctl":payload,
            "command_ok":response["ok"],
            "notice":"SMART-Werte können fehlen. M.2-Bauform nicht verifiziert. Keine Selbsttests oder Schreibbefehle."}

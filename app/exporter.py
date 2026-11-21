"""Local case export with conservative optional anonymization.

Anonymization deliberately omits free-text diagnostics and raw controller
identification: arbitrary text can contain identifiers that key-based
redaction cannot reliably remove.
"""
import json
from copy import deepcopy
from datetime import datetime, timezone
from pathlib import Path

PRIVATE_KEYS={"serial","serialnumber","sn","device_path","path","device",
              "hostname","computername","username","pnpdeviceid","wwn",
              "eui64","nguid","uuid","subnqn","hostnqn","hostid"}
FREE_TEXT_KEYS={"stdout","stderr","system","pci","usb","processor","os",
                "source","notes","header_hex","error","evidence"}
RAW_SECTIONS={"identify","errors","hardware","usb_diagnostics","inventory"}
MASK="[ausgeblendet]"

def redact(value):
    if isinstance(value,dict):
        result={}
        for key,item in value.items():
            name=str(key).lower()
            if name in PRIVATE_KEYS or name in FREE_TEXT_KEYS or name in RAW_SECTIONS:
                result[key]=MASK
            else:
                result[key]=redact(item)
        return result
    if isinstance(value,list): return [redact(v) for v in value]
    if isinstance(value,str):
        # Unknown free text may still contain personal data. Do not promise full anonymization.
        return value[:4000]
    return value

def export_json(path,payload,anonymize=True):
    data=redact(deepcopy(payload)) if anonymize else deepcopy(payload)
    document={"format":"NVMe Recovery Studio AI - Fallakte",
              "exported":datetime.now(timezone.utc).isoformat(),
              "anonymized":bool(anonymize),
              "privacy_notice":("Rohdaten und bekannte Kennungen wurden entfernt. "
                                "Vollständige Anonymität kann nicht garantiert werden."
                                if anonymize else "Enthält möglicherweise personenbezogene Geräteinformationen."),
              "data":data}
    Path(path).write_text(json.dumps(document,indent=2,ensure_ascii=False),encoding="utf-8")

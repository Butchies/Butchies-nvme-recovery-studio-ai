"""Case export with optional redaction; local files only."""
import json
from copy import deepcopy
from datetime import datetime, timezone
from pathlib import Path

PRIVATE_KEYS={"serial","serialnumber","device_path","path","hostname","computername","username"}
def redact(value):
    if isinstance(value,dict):
        return {k:("[ausgeblendet]" if k.lower() in PRIVATE_KEYS else redact(v)) for k,v in value.items()}
    if isinstance(value,list): return [redact(v) for v in value]
    return value

def export_json(path, payload, anonymize=True):
    data=deepcopy(payload)
    if anonymize: data=redact(data)
    document={"format":"NVMe Recovery Studio AI – Fallakte",
              "exported":datetime.now(timezone.utc).isoformat(),
              "anonymized":anonymize,"data":data}
    Path(path).write_text(json.dumps(document,indent=2,ensure_ascii=False),encoding="utf-8")

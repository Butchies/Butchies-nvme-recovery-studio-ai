"""Read-only safety classification for selected block devices.

This module never grants permission to flash, erase or overwrite disks.
It records reasons why a selected drive must not be modified.
"""
import json
import re
import sys
from datetime import datetime, timezone

import diagnostics

def classify(device):
    result={"checked":datetime.now(timezone.utc).isoformat(),
            "device":device.get("path"),"protected":True,
            "writes_allowed":False,"reasons":[],"observations":[]}
    path=str(device.get("path") or "")
    if sys.platform=="win32":
        if device.get("bus")!="USB":
            result["reasons"].append("Windows-Modus: interne Laufwerke grundsätzlich gesperrt.")
        else:
            result["reasons"].append("Windows-Modus: nur lesende Diagnose, keine Rohschreibzugriffe implementiert.")
        result["observations"].append("USB-Bridge kann NVMe-Controllerdetails verbergen.")
        return result
    if not sys.platform.startswith("linux"):
        result["reasons"].append("Nicht unterstütztes Betriebssystem.")
        return result
    if not re.fullmatch(r"/dev/(?:nvme[0-9]+n[0-9]+|sd[a-z]+)",path):
        result["reasons"].append("Gerätepfad ist nicht als physischer Datenträger erkennbar.")
        return result
    response=diagnostics.run(["lsblk","-J","-b","-o","NAME,PATH,TYPE,TRAN,SERIAL,MODEL,SIZE,MOUNTPOINTS,PKNAME"])
    if not response["ok"]:
        result["reasons"].append("Datenträgerstruktur konnte nicht ermittelt werden.")
        return result
    try: tree=json.loads(response["stdout"]).get("blockdevices",[])
    except (ValueError,TypeError):
        result["reasons"].append("Datenträgerstruktur unlesbar.")
        return result
    def walk(items):
        for item in items:
            yield item
            yield from walk(item.get("children") or [])
    roots=[item for item in tree if item.get("path")==path and item.get("type")=="disk"]
    if len(roots)!=1:
        result["reasons"].append("Datenträger nicht eindeutig identifiziert.")
        return result
    root=roots[0]
    if str(root.get("serial") or "").strip()!=str(device.get("serial") or "").strip() or int(root.get("size") or 0)!=int(device.get("size") or 0):
        result["reasons"].append("Geräteidentität hat sich seit Auswahl verändert.")
        return result
    mounts=[]
    for node in walk([root]):
        mounts.extend(m for m in (node.get("mountpoints") or []) if m)
    result["observations"].append({"mountpoints":sorted(set(mounts))})
    if mounts:
        result["reasons"].append("Laufwerk enthält eingehängte Dateisysteme; darunter könnte das System oder Bootmedium sein.")
    else:
        result["reasons"].append("Keine Mountpoints erkannt. Das allein beweist NICHT, dass das Laufwerk kein System-/Bootmedium ist.")
    result["reasons"].append("Schreiboperationen sind in dieser Version vollständig gesperrt.")
    return result

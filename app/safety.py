"""Read-only safety classification for selected block devices.

This module never grants permission to flash, erase or overwrite disks.
It records reasons why a selected drive must not be modified.
"""
import json
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
    if not path.startswith("/dev/") or path == "/dev/":
        result["reasons"].append("Ungültiger Gerätepfad.")
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
    result["observations"].append({"transport":root.get("tran"),"model":root.get("model"),"type":root.get("type")})
    if str(root.get("serial") or "").strip()!=str(device.get("serial") or "").strip() or int(root.get("size") or 0)!=int(device.get("size") or 0):
        result["reasons"].append("Geräteidentität hat sich seit Auswahl verändert.")
        return result
    mounts=[]
    child_paths=[]
    for node in walk([root]):
        mounts.extend(m for m in (node.get("mountpoints") or []) if m)
        if node.get("path"): child_paths.append(str(node["path"]))
    findmnt=diagnostics.run(["findmnt","-J","-o","SOURCE,TARGET"])
    active_mounts=[]
    if findmnt["ok"]:
        try:
            def mounts_walk(items):
                for entry in items:
                    yield entry
                    yield from mounts_walk(entry.get("children") or [])
            entries=json.loads(findmnt["stdout"]).get("filesystems",[])
            active_mounts=[{"source":x.get("source"),"target":x.get("target")}
                           for x in mounts_walk(entries)
                           if any(str(x.get("source") or "").startswith(p) for p in child_paths)]
        except (ValueError,TypeError): pass
    result["observations"].append({"active_mounts":active_mounts})
    if any(x.get("target") in ("/","/boot","/boot/efi","/efi") for x in active_mounts):
        result["reasons"].append("Aktives System- oder Boot-Dateisystem erkannt.")
    swaps=diagnostics.run(["swapon","--noheadings","--raw","--show=NAME"])
    swap_paths=swaps["stdout"].splitlines() if swaps["ok"] else []
    active_swaps=[x for x in swap_paths if any(x.startswith(p) for p in child_paths)]
    if active_swaps: result["reasons"].append("Aktiver Swap auf diesem Laufwerk erkannt.")
    result["observations"].append({"active_swap":active_swaps})
    if not findmnt["ok"] or not swaps["ok"]:
        result["reasons"].append("System-/Swap-Abfrage unvollständig; keine Entwarnung möglich.")
    result["observations"].append({"mountpoints":sorted(set(mounts))})
    if mounts:
        result["reasons"].append("Laufwerk enthält eingehängte Dateisysteme; darunter könnte das System oder Bootmedium sein.")
    else:
        result["reasons"].append("Keine Mountpoints erkannt. Das allein beweist NICHT, dass das Laufwerk kein System-/Bootmedium ist.")
    result["reasons"].append("Schreiboperationen sind in dieser Version vollständig gesperrt.")
    return result

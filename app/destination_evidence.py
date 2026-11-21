"""Conservative read-only Linux destination ancestry evidence.

Never grants permission to image, flash or modify a block device.
"""
import json
import os
import subprocess
import sys

def _run(args):
    try:
        p=subprocess.run(args,capture_output=True,text=True,timeout=8)
        return json.loads(p.stdout) if p.returncode==0 else None
    except (OSError,ValueError,subprocess.TimeoutExpired):
        return None

def _canonical(path):
    return os.path.realpath(path) if isinstance(path,str) and path.startswith("/dev/") else ""

def destination_evidence(directory,source):
    result={"supported":sys.platform.startswith("linux"),
            "source_disk":source,"destination_mount":None,
            "destination_source":None,"destination_ancestors":[],
            "same_device_detected":False,"separation_verified":False,
            "warnings":[]}
    warnings=result["warnings"]
    if not result["supported"]:
        warnings.append("Mount-Abstammung wird nur unter Linux untersucht.")
        return result
    mount=_run(["findmnt","-J","-T",str(directory),"-o","SOURCE,TARGET,FSTYPE"])
    entries=(mount or {}).get("filesystems") or []
    if not entries:
        warnings.append("Ziel-Mount konnte nicht bestimmt werden.")
        return result
    item=entries[0]
    result["destination_mount"]=item.get("target")
    result["destination_source"]=item.get("source")
    source_path=_canonical(item.get("source"))
    selected=_canonical(source)
    if not source_path or not selected:
        warnings.append("Quelle oder Ziel liegt nicht auf einem eindeutig erkennbaren /dev-Gerät.")
        return result
    tree=_run(["lsblk","-J","-p","-o","PATH,TYPE"])
    if not tree or not isinstance(tree.get("blockdevices"),list):
        warnings.append("Blockgerätehierarchie konnte nicht gelesen werden.")
        return result
    parents={}
    def walk(items,parent=None):
        for node in items or []:
            if not isinstance(node,dict):continue
            path=_canonical(node.get("path"))
            if path:
                parents.setdefault(path,set())
                if parent:parents[path].add(parent)
            walk(node.get("children"),path or parent)
    walk(tree["blockdevices"])
    if source_path not in parents:
        warnings.append("Zielgerät ist nicht vollständig in lsblk auflösbar.")
        return result
    seen=set()
    pending=[source_path]
    while pending and len(seen)<512:
        current=pending.pop()
        if current in seen:continue
        seen.add(current)
        pending.extend(parents.get(current,()))
    result["destination_ancestors"]=sorted(seen)
    if selected in seen:
        result["same_device_detected"]=True
        warnings.append("KRITISCH: Ziel verwendet den ausgewählten Quelldatenträger.")
    if len(seen)>=512:
        warnings.append("Blockgerätegraph ist zu groß oder unvollständig.")
    warnings.append("Die Erkennung bleibt unvollständig bei RAID, LVM, Loop, Netzwerk, Bind-Mounts und Verschlüsselung. Keine Freigabe.")
    return result

"""Read-only Linux mount ancestry evidence for imaging planning.

Informational only: stacked, remote, encrypted and virtual filesystems
can obscure the actual backing disk. Never authorizes imaging.
"""
import json
import subprocess
import sys

def _run(args):
    try:
        p=subprocess.run(args,capture_output=True,text=True,timeout=8)
        return json.loads(p.stdout) if p.returncode==0 else None
    except (OSError,ValueError,subprocess.TimeoutExpired):
        return None

def destination_evidence(directory,source):
    result={"supported":sys.platform.startswith("linux"),
            "source_disk":source,"destination_mount":None,
            "destination_source":None,"destination_ancestors":[],
            "same_device_detected":False,"separation_verified":False,
            "warnings":[]}
    if not result["supported"]:
        result["warnings"].append("Mount-Abstammung wird nur unter Linux untersucht.")
        return result
    mount=_run(["findmnt","-J","-T",str(directory),"-o","SOURCE,TARGET,FSTYPE"])
    entries=(mount or {}).get("filesystems") or []
    if not entries:
        result["warnings"].append("Ziel-Mount konnte nicht bestimmt werden.")
        return result
    item=entries[0]
    result["destination_mount"]=item.get("target")
    result["destination_source"]=item.get("source")
    source_path=str(item.get("source") or "")
    if not source_path.startswith("/dev/"):
        result["warnings"].append("Ziel liegt auf nicht eindeutig zuordenbarem Dateisystem.")
        return result
    # lsblk tree can expose partitions and device-mapper layers.
    tree=_run(["lsblk","-J","-p","-o","PATH,PKNAME,TYPE"])
    nodes=[]
    def walk(items):
        for node in items or []:
            if isinstance(node,dict):
                nodes.append(node)
                walk(node.get("children"))
    walk((tree or {}).get("blockdevices"))
    by_path={str(x.get("path")):x for x in nodes if x.get("path")}
    cursor=source_path
    visited=set()
    while cursor and cursor not in visited and len(visited)<32:
        visited.add(cursor)
        result["destination_ancestors"].append(cursor)
        parent=by_path.get(cursor,{}).get("pkname")
        cursor=str(parent) if parent else ""
    if source in visited:
        result["same_device_detected"]=True
        result["warnings"].append("ACHTUNG: Ziel liegt auf dem ausgewählten Quelldatenträger.")
    if source_path not in by_path:
        result["warnings"].append("Zielgerät ist nicht vollständig in lsblk auflösbar.")
    result["warnings"].append("Abstammung ist nur ein Hinweis. LVM, RAID, Loop, Netzwerk und Verschlüsselung können weitere Abhängigkeiten verbergen.")
    return result

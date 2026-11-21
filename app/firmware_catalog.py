"""Offline firmware catalog with strict metadata validation.

Catalog entries are untrusted research notes, NOT flashing authorizations.
No network requests or device writes.
"""
import json
import os
import stat
import re
from pathlib import Path

MAX_CATALOG_BYTES=2*1024*1024
HEX64=re.compile(r"^[a-fA-F0-9]{64}$")
HEX4=re.compile(r"^[a-fA-F0-9]{4}$")

def load(path):
    file=Path(path)
    try:
        if file.is_symlink():
            return {"error":"Symlinks sind nicht erlaubt."}
        flags=os.O_RDONLY|getattr(os,"O_BINARY",0)|getattr(os,"O_NOFOLLOW",0)
        fd=os.open(file,flags)
        with os.fdopen(fd,"rb") as stream:
            before=os.fstat(stream.fileno())
            if not stat.S_ISREG(before.st_mode) or before.st_size>MAX_CATALOG_BYTES:
                return {"error":"Katalog muss eine reguläre Datei bis 2 MiB sein."}
            raw=stream.read(MAX_CATALOG_BYTES+1)
            after=os.fstat(stream.fileno())
            if len(raw)>MAX_CATALOG_BYTES:
                return {"error":"Katalogdatei überschreitet 2 MiB."}
            if (before.st_size!=len(raw) or before.st_size!=after.st_size or
                before.st_mtime_ns!=after.st_mtime_ns or before.st_ctime_ns!=after.st_ctime_ns):
                return {"error":"Katalogdatei wurde während des Lesens verändert."}
        data=json.loads(raw.decode("utf-8"))
    except (OSError,ValueError,UnicodeError) as exc:
        return {"error":"Katalog kann nicht sicher gelesen werden: "+str(exc)}
    if not isinstance(data,dict) or data.get("schema")!=1 or not isinstance(data.get("entries"),list):
        return {"error":"Ungültiges Katalogformat (schema 1 erwartet)."}
    if len(data["entries"])>1000:
        return {"error":"Zu viele Einträge (maximal 1000)."}
    entries=[]
    for index,item in enumerate(data["entries"]):
        if not isinstance(item,dict):
            return {"error":f"Eintrag {index+1} ist kein Objekt."}
        sha=item.get("sha256")
        vendor=item.get("pci_vendor_id")
        device=item.get("pci_device_id")
        if not isinstance(sha,str) or not HEX64.fullmatch(sha):
            return {"error":f"Eintrag {index+1}: ungültiger SHA-256-Wert."}
        if not all(isinstance(x,str) and HEX4.fullmatch(x) for x in (vendor,device)):
            return {"error":f"Eintrag {index+1}: PCI Vendor/Device ID muss vier Hexzeichen haben."}
        entries.append({"sha256":sha.lower(),"pci_vendor_id":vendor.lower(),
                        "pci_device_id":device.lower(),
                        "model":str(item.get("model") or "")[:160],
                        "firmware_revision":str(item.get("firmware_revision") or "")[:80],
                        "source":str(item.get("source") or "")[:400],
                        "notes":str(item.get("notes") or "")[:500]})
    return {"schema":1,"entries":entries,"count":len(entries),
            "warning":"Katalogdaten sind nicht authentifiziert und keine Flash-Freigabe."}

def lookup(catalog, fingerprint):
    if "error" in catalog: return {"error":catalog["error"]}
    sha=str(fingerprint.get("sha256") or "").lower()
    if not HEX64.fullmatch(sha): return {"error":"Zuerst eine Firmwaredatei erfolgreich analysieren."}
    matches=[x for x in catalog.get("entries",[]) if x["sha256"]==sha]
    return {"sha256":sha,"hash_matches":matches,
            "status":"Hash im lokalen Katalog vorhanden" if matches else "Kein Hash-Treffer",
            "authenticity_verified":False,"controller_compatibility_verified":False,
            "flashing_allowed":False,
            "warning":"Ein Hash-Treffer belegt nur Übereinstimmung mit einem lokalen Katalogeintrag, nicht Echtheit, Sicherheit oder Hardware-Kompatibilität."}

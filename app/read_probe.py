"""Bounded read-only probe of a selected direct Linux NVMe namespace.

No image, recovery, firmware or write operation is performed.
"""
import os
import re
import sys
import json
import subprocess
import time
import struct

PATH=re.compile(r"/dev/nvme[0-9]+n[0-9]+")
MAX_SAMPLE=4096

def probe(device):
    result={"mode":"bounded_read_only_probe","device_writes_allowed":False,
            "bytes_requested":0,"bytes_read":0,"probe_success":False}
    if not sys.platform.startswith("linux") or not isinstance(device,dict):
        return dict(result,error="Nur direkte NVMe-Namespaces unter Linux unterstützt.")
    path=str(device.get("path") or "")
    if not PATH.fullmatch(path) or device.get("bus")!="PCIe/NVMe" or not device.get("nvme"):
        return dict(result,error="Kein direktes NVMe-Namespace.")
    try:
        proc=subprocess.run(["lsblk","-J","-b","-d","-o","PATH,TYPE,TRAN,SERIAL,SIZE"],
                            capture_output=True,text=True,timeout=8,check=False)
        rows=json.loads(proc.stdout).get("blockdevices",[]) if proc.returncode==0 else []
    except (OSError,ValueError,TypeError,subprocess.TimeoutExpired):
        rows=[]
    rows=[r for r in rows if isinstance(r,dict) and r.get("path")==path and r.get("type")=="disk"]
    if len(rows)!=1 or str(rows[0].get("tran") or "").lower()=="usb":
        return dict(result,error="Gerät nicht mehr eindeutig vorhanden.")
    try:
        size=int(device.get("size") or 0)
        live_size=int(rows[0].get("size") or 0)
    except (ValueError,TypeError,OverflowError):
        return dict(result,error="Kapazität nicht prüfbar.")
    serial=str(device.get("serial") or "").strip()
    if not serial or serial!=str(rows[0].get("serial") or "").strip() or size<=0 or size!=live_size:
        return dict(result,error="Geräteidentität hat sich verändert.")
    # This is intentionally a single small read, not a surface scan or imaging loop.
    result["bytes_requested"]=min(size,MAX_SAMPLE)
    flags=os.O_RDONLY|getattr(os,"O_CLOEXEC",0)|getattr(os,"O_NONBLOCK",0)
    started=time.monotonic()
    fd=None
    try:
        fd=os.open(path,flags)
        chunk=os.pread(fd,result["bytes_requested"],0)
        result["bytes_read"]=len(chunk)
        result["sector_evidence"]=interpret_header(chunk)
        result["probe_success"]=len(chunk)==result["bytes_requested"]
        result["elapsed_seconds"]=round(time.monotonic()-started,3)
        result["observation"]="Anfang des Datenträgers lesbar." if result["probe_success"] else "Unvollständige Leseantwort."
    except OSError as exc:
        result["error"]=f"Lesefehler ({exc.errno}): {exc.strerror}"
    finally:
        if fd is not None: os.close(fd)
    result["warning"]="Eine erfolgreiche Einzelprobe beweist nicht, dass andere Sektoren lesbar sind. Bei instabilen SSDs können auch Leseversuche problematisch sein."
    return result

def interpret_header(data):
    """Read-only interpretation of already sampled bytes; no further I/O."""
    evidence={"bytes_available":len(data),"partition_table":"unknown",
              "filesystem":"unknown","metadata_only":True}
    if len(data)<512:
        evidence["warning"]="Weniger als ein vollständiger 512-Byte-Sektor gelesen."
        return evidence
    signature=data[510:512]==bytes((0x55,0xaa))
    evidence["mbr_signature_present"]=signature
    if signature:
        entries=[]
        for index in range(4):
            entry=data[446+index*16:462+index*16]
            ptype=entry[4]
            start=struct.unpack_from("<I",entry,8)[0]
            count=struct.unpack_from("<I",entry,12)[0]
            if ptype and count:
                entries.append({"index":index+1,"type_hex":format(ptype,"#04x"),
                                "start_lba":start,"sectors":count})
        evidence["partition_entries"]=entries
        evidence["partition_table"]="protective_mbr" if any(e["type_hex"]=="0xee" for e in entries) else "mbr_possible"
        if evidence["partition_table"]=="protective_mbr":
            evidence["warning"]="GPT möglich; der GPT-Header bei LBA 1 wurde nicht separat geprüft."
    if data[3:11]==b"NTFS    ":
        evidence["filesystem"]="ntfs_boot_sector_possible"
    elif data[3:11]==b"EXFAT   ":
        evidence["filesystem"]="exfat_boot_sector_possible"
    elif data[54:62] in (b"FAT12   ",b"FAT16   ") or data[82:90]==b"FAT32   ":
        evidence["filesystem"]="fat_boot_sector_possible"
    evidence["note"]="Signaturen sind Hinweise, keine Integritätsprüfung."
    return evidence

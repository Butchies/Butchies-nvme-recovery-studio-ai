"""Linux PCIe NVMe controller identification using read-only sysfs metadata.

This does not enter vendor recovery mode or authorize firmware operations.
"""
import re
import json
import subprocess
import sys
from pathlib import Path

NAMESPACE=re.compile(r"nvme([0-9]+)n[0-9]+")
HEX=re.compile(r"0x[0-9a-fA-F]{4,8}")

def _read(path):
    try:
        return path.read_text(encoding="ascii").strip()[:128]
    except (OSError,UnicodeError):
        return None

def report(device):
    result={"mode":"read_only_controller_identity","controller_verified":False,
            "firmware_write_allowed":False,"vendor_recovery_supported":False}
    if not sys.platform.startswith("linux") or not isinstance(device,dict):
        return dict(result,error="Controllerzuordnung nur für direkte Linux-NVMe-Geräte.")
    path=str(device.get("path") or "")
    match=NAMESPACE.fullmatch(Path(path).name) if path.startswith("/dev/") else None
    if not match or device.get("bus")!="PCIe/NVMe" or not device.get("nvme"):
        return dict(result,error="Kein direktes NVMe-Namespace ausgewählt.")
    # Revalidate live namespace identity before associating a PCI controller.
    try:
        proc=subprocess.run(["lsblk","-J","-b","-d","-o","NAME,PATH,TYPE,TRAN,SERIAL,SIZE"],capture_output=True,text=True,timeout=10,check=False)
        rows=json.loads(proc.stdout).get("blockdevices",[]) if proc.returncode==0 else []
    except (OSError,ValueError,TypeError,subprocess.TimeoutExpired):
        rows=[]
    matches=[x for x in rows if isinstance(x,dict) and x.get("path")==path and x.get("type")=="disk"]
    if len(matches)!=1 or str(matches[0].get("tran") or "").lower()=="usb":
        return dict(result,error="Namespace nicht mehr eindeutig vorhanden.")
    current=matches[0]
    try:
        same_size=int(current.get("size") or 0)==int(device.get("size") or 0)>0
    except (ValueError,TypeError,OverflowError):
        same_size=False
    if not same_size or not str(device.get("serial") or "").strip() or str(current.get("serial") or "").strip()!=str(device.get("serial") or "").strip():
        return dict(result,error="Geräteidentität hat sich verändert.")
    name="nvme"+match.group(1)
    controller=Path("/sys/class/nvme")/name
    try:
        actual=controller.resolve(strict=True)
        pci=(actual/"device").resolve(strict=True)
        if not str(actual).startswith("/sys/") or not str(pci).startswith("/sys/"):
            return dict(result,error="Sysfs-Zuordnung ungültig.")
    except OSError:
        return dict(result,error="NVMe-Controller nicht in sysfs vorhanden.")
    vendor=_read(pci/"vendor")
    product=_read(pci/"device")
    subsystem_vendor=_read(pci/"subsystem_vendor")
    subsystem_device=_read(pci/"subsystem_device")
    result.update(controller=name,pci_address=pci.name,
                  pci_vendor_id=vendor,pci_device_id=product,
                  pci_subsystem_vendor_id=subsystem_vendor,
                  pci_subsystem_device_id=subsystem_device,
                  controller_model=_read(controller/"model"),
                  controller_serial=_read(controller/"serial"),
                  firmware_revision=_read(controller/"firmware_rev"),
                  driver=_read(pci/"driver_override"))
    result["controller_verified"]=bool(vendor and product and HEX.fullmatch(vendor) and HEX.fullmatch(product) and result.get("controller_serial")==str(device.get("serial") or "").strip())
    result["note"]="PCI-IDs identifizieren Hardware, aber keine sichere Firmware oder Vendor-Recovery-Befehle."
    return result

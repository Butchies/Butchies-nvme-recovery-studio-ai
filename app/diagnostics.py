"""Read-only host and NVMe diagnostics. No device writes."""
import json
import os
import re
import platform
import shutil
import subprocess
import sys
from datetime import datetime, timezone

def run(argv, timeout=12):
    try:
        p=subprocess.run(argv, capture_output=True, text=True, timeout=timeout,
                         creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0), check=False)
        return {"ok":p.returncode==0, "stdout":p.stdout[:200000], "stderr":p.stderr[:2000]}
    except (OSError,subprocess.TimeoutExpired) as exc:
        return {"ok":False, "stdout":"", "stderr":str(exc)}

def hardware():
    info={"os":platform.platform(),"architecture":platform.machine(),
          "processor":platform.processor(),"cpu_count":os.cpu_count(),
          "python":platform.python_version(),"timestamp":datetime.now(timezone.utc).isoformat()}
    if sys.platform.startswith("linux"):
        info["memory"]=run(["free","-h"])["stdout"]
        info["pci"]=run(["lspci","-nn"])["stdout"] if shutil.which("lspci") else "lspci nicht vorhanden"
        info["usb"]=run(["lsusb"])["stdout"] if shutil.which("lsusb") else "lsusb nicht vorhanden"
    elif sys.platform=="win32":
        script="Get-CimInstance Win32_Processor | Select-Object Name,NumberOfCores; Get-CimInstance Win32_ComputerSystem | Select-Object Manufacturer,Model,TotalPhysicalMemory"
        info["system"]=run(["powershell.exe","-NoProfile","-NonInteractive","-Command",script])["stdout"]
    return info

def nvme_report(device):
    """Linux only. Reject partitions, USB targets and arbitrary paths."""
    if not sys.platform.startswith("linux"): return {"error":"NVMe-Rohdiagnose nur unter Linux"}
    if not isinstance(device,dict): return {"error":"Ungültige Geräteauswahl"}
    path=str(device.get("path",""))
    if device.get("bus")!="PCIe/NVMe" or not re.fullmatch(r"/dev/nvme[0-9]+n[0-9]+",path):
        return {"error":"Nur eindeutig identifizierte direkte NVMe-Namespaces"}
    if not shutil.which("nvme"): return {"error":"nvme-cli fehlt"}
    # Verify the selected namespace is still the same device and is not a USB bridge.
    check=run(["lsblk","-J","-b","-d","-o","NAME,SERIAL,MODEL,SIZE,TRAN,TYPE,PATH"])
    try: rows=json.loads(check["stdout"]).get("blockdevices",[]) if check["ok"] else []
    except (ValueError,TypeError): rows=[]
    matches=[x for x in rows if isinstance(x,dict) and x.get("path")==path and x.get("type")=="disk" and str(x.get("name","")).startswith("nvme")]
    if len(matches)!=1: return {"error":"Gerät nicht mehr eindeutig vorhanden"}
    current=matches[0]
    try: same_size=int(current.get("size") or 0)==int(device.get("size") or 0)>0
    except (ValueError,TypeError,OverflowError): same_size=False
    if (not str(device.get("serial") or "").strip() or
        str(current.get("serial") or "").strip()!=str(device.get("serial") or "").strip() or
        not same_size or str(current.get("tran") or "").lower()=="usb"):
        return {"error":"Geräteidentität seit Auswahl verändert"}
    result={"device":path,"timestamp":datetime.now(timezone.utc).isoformat()}
    for key,args in {
        "smart":["nvme","smart-log",path,"-o","json"],
        "identify":["nvme","id-ctrl",path,"-o","json"],
        "errors":["nvme","error-log",path,"-e","16","-o","json"]
    }.items():
        response=run(args)
        if response["ok"]:
            try: result[key]=json.loads(response["stdout"])
            except ValueError: result[key]={"error":"Ungültige JSON-Antwort"}
        else: result[key]={"error":response["stderr"] or "Abfrage nicht möglich"}
    return result

def usb_report(device):
    """Read-only Windows USB inventory; does not claim NVMe passthrough."""
    if sys.platform!="win32":
        return {"error":"USB-Bridge-Bericht derzeit nur unter Windows verfügbar"}
    if not isinstance(device,dict) or device.get("bus")!="USB":
        return {"error":"Nur USB-Geräte sind für diese Windows-Ansicht zulässig"}
    path=str(device.get("path") or "")
    # Validate the canonical Win32 physical-drive path without a permissive regex.
    prefix=chr(92)*2+"."+chr(92)+"PHYSICALDRIVE"
    if not path.upper().startswith(prefix) or not path[len(prefix):].isdigit():
        return {"error":"Ungültiger Windows-Gerätepfad"}
    number=int(path.rsplit("PHYSICALDRIVE",1)[-1]) if "PHYSICALDRIVE" in path else int(path.upper().rsplit("PHYSICALDRIVE",1)[-1])
    script=(
        "$ErrorActionPreference='Stop'; "
        "$d=Get-CimInstance Win32_DiskDrive -Filter 'Index="+str(number)+"'; "
        "if (-not $d) { throw 'Gerät nicht vorhanden' }; "
        "if ($d.InterfaceType -ne 'USB' -and $d.PNPDeviceID -notmatch '^USBSTOR|^USB\\\\') { throw 'Gerät ist nicht USB' }; "
        "$d | Select-Object Index,Model,SerialNumber,Size,InterfaceType,PNPDeviceID,Status | ConvertTo-Json -Compress"
    )
    response=run(["powershell.exe","-NoProfile","-NonInteractive","-Command",script])
    if not response["ok"]:
        return {"error":response["stderr"] or "USB-Gerät nicht auslesbar"}
    try: details=json.loads(response["stdout"])
    except (ValueError,TypeError): return {"error":"Ungültige Geräteantwort"}
    if not isinstance(details,dict):return {"error":"Unerwartete Geräteantwort"}
    try: same_size=int(details.get("Size") or 0)==int(device.get("size") or 0)>0
    except (ValueError,TypeError,OverflowError):same_size=False
    if (not str(device.get("serial") or "").strip() or
        str(details.get("SerialNumber") or "").strip()!=str(device.get("serial") or "").strip() or not same_size):
        return {"error":"Geräteidentität hat sich verändert"}
    return {"device":path,"transport":"USB","inventory":details,
            "nvme_verified":False,
            "limitations":["USB-Gerät erkannt, NVMe hinter der Bridge nicht bestätigt.",
                           "Kein NVMe-Admin-Passthrough und keine Firmwareoperation verfügbar.",
                           "Bei fehlenden NVMe-Daten eine geeignete Bridge oder direkten PCIe-Zugriff verwenden."]}

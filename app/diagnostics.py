"""Read-only host and NVMe diagnostics. No device writes."""
import json
import os
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
    path=str(device.get("path",""))
    if device.get("bus")!="PCIe/NVMe" or not __import__("re").fullmatch(r"/dev/nvme[0-9]+n[0-9]+",path):
        return {"error":"Nur eindeutig identifizierte direkte NVMe-Namespaces"}
    if not shutil.which("nvme"): return {"error":"nvme-cli fehlt"}
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

"""Read-only imaging feasibility planner. Does not open devices or create images.

No result from this module is permission to perform device I/O.
"""
import os
import sys
import destination_evidence
from pathlib import Path
from datetime import datetime, timezone

MIN_RESERVE=1024*1024*1024

def plan(device, destination_directory):
    result={"created":datetime.now(timezone.utc).isoformat(),
            "mode":"planning_only","source_reads_performed":False,
            "image_created":False,"device_writes_allowed":False,
            "ready_to_image":False,"issues":[]}
    issues=result["issues"]
    if not isinstance(device,dict):
        issues.append("Keine gültige Geräteauswahl.")
        return result
    source=str(device.get("path") or "")
    serial=str(device.get("serial") or "").strip()
    size=device.get("size")
    try: size=int(size)
    except (TypeError,ValueError): size=0
    if not source or not serial or size<=0:
        issues.append("Gerätepfad, Seriennummer oder Kapazität fehlt.")
    if sys.platform=="win32" and device.get("bus")!="USB":
        issues.append("Windows erlaubt nur extern angeschlossene USB-Ziele.")
    if sys.platform=="win32" and device.get("nvme") is not True:
        issues.append("NVMe hinter der USB-Bridge ist nicht bestätigt.")
    directory=Path(destination_directory)
    if not directory.is_dir() or directory.is_symlink():
        issues.append("Zielverzeichnis ist nicht vorhanden oder ein Symlink.")
    else:
        try:
            target=directory.resolve(strict=True)
            if sys.platform.startswith("linux"):
                # A directory on the selected source would overwrite recoverable data.
                # Detecting this safely requires complete block-device dependency mapping.
                evidence=destination_evidence.destination_evidence(str(target),source)
                result["destination_evidence"]=evidence
                if evidence["same_device_detected"]:
                    issues.append("KRITISCH: Das Zielverzeichnis liegt auf dem ausgewählten Quelldatenträger.")
                issues.append("Ziel-Datenträgerzuordnung unter Linux nicht abschließend verifiziert.")
            stats=os.statvfs(str(target)) if hasattr(os,"statvfs") else None
            if stats is not None:
                free=stats.f_bavail*stats.f_frsize
                result["destination_free_bytes"]=free
                if size and free<size+MIN_RESERVE:
                    issues.append("Zu wenig freier Speicher für ein vollständiges Rohabbild plus 1 GiB Reserve.")
            else:
                import shutil
                free=shutil.disk_usage(str(target)).free
                result["destination_free_bytes"]=free
                if size and free<size+MIN_RESERVE:
                    issues.append("Zu wenig freier Speicher für ein vollständiges Rohabbild plus 1 GiB Reserve.")
        except OSError:
            issues.append("Freier Speicher des Zielverzeichnisses konnte nicht ermittelt werden.")
    result["source_path"]=source
    result["source_size_bytes"]=size
    result["destination_directory"]=str(directory)
    result["estimated_image_bytes"]=size
    result["issues"].append("Quell- und Zieldatenträger-Trennung noch nicht nachgewiesen.")
    result["warning"]="Nur Kapazitätsplanung. Keine Kopie, keine Geräteöffnung und keine Freigabe."
    return result

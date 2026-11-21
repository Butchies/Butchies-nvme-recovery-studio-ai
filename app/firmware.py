"""Offline firmware file inspection. NEVER executes, loads, or flashes firmware.

A hash and byte-pattern preview cannot establish controller compatibility.
"""
import hashlib
import os
from pathlib import Path
from datetime import datetime, timezone

MAX_BYTES=256*1024*1024
CHUNK=1024*1024

def inspect(path):
    file=Path(path)
    if not file.is_file() or file.is_symlink():
        return {"error":"Nur vorhandene reguläre Dateien (keine Symlinks) zulässig."}
    stat=file.stat()
    if stat.st_size<=0 or stat.st_size>MAX_BYTES:
        return {"error":"Dateigröße außerhalb des Analysebereichs (1 Byte bis 256 MiB)."}
    digest=hashlib.sha256()
    preview=b""
    count=0
    with file.open("rb") as stream:
        opened=os.fstat(stream.fileno())
        if not os.path.isfile(file):
            return {"error":"Datei nicht mehr vorhanden."}
        if opened.st_size!=stat.st_size or opened.st_dev!=stat.st_dev or opened.st_ino!=stat.st_ino:
            return {"error":"Datei wurde zwischen Auswahl und Öffnen verändert."}
        while True:
            block=stream.read(CHUNK)
            if not block: break
            if count==0: preview=block[:64]
            count+=len(block)
            if count>MAX_BYTES: return {"error":"Datei überschreitet die Größenbegrenzung."}
            digest.update(block)
        final=os.fstat(stream.fileno())
    if final.st_size!=stat.st_size or count!=stat.st_size or final.st_mtime_ns!=stat.st_mtime_ns:
        return {"error":"Datei wurde während des Lesens verändert; Hash nicht vertrauenswürdig."}
    return {
        "timestamp":datetime.now(timezone.utc).isoformat(),
        "filename":file.name,"size_bytes":count,
        "sha256":digest.hexdigest(),"header_hex":preview.hex(" "),
        "controller_compatibility":"NICHT VERIFIZIERT",
        "signature_verified":False,
        "firmware_flashing_enabled":False,
        "notice":"Nur Dateianalyse. Dateiendung, Header und Hash beweisen keine Firmware-Echtheit oder Controller-Kompatibilität."
    }

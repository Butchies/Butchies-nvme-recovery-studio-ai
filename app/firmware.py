"""Offline firmware inspection. No execution, loading, or flashing."""
import hashlib
import os
import stat
from pathlib import Path
from datetime import datetime, timezone

MAX_BYTES=256*1024*1024
CHUNK=1024*1024

def inspect(path):
    file=Path(path)
    flags=os.O_RDONLY|getattr(os,"O_BINARY",0)|getattr(os,"O_NOFOLLOW",0)
    try:
        if file.is_symlink():
            return {"error":"Symlinks sind nicht erlaubt."}
        fd=os.open(file,flags)
        with os.fdopen(fd,"rb") as stream:
            opened=os.fstat(stream.fileno())
            if not stat.S_ISREG(opened.st_mode):
                return {"error":"Nur reguläre Firmwaredateien sind erlaubt."}
            if not 0<opened.st_size<=MAX_BYTES:
                return {"error":"Dateigröße außerhalb des Analysebereichs (1 Byte bis 256 MiB)."}
            digest=hashlib.sha256()
            preview=b""
            count=0
            while True:
                block=stream.read(min(CHUNK,MAX_BYTES-count+1))
                if not block:break
                if count==0:preview=block[:64]
                count+=len(block)
                if count>MAX_BYTES:
                    return {"error":"Datei überschreitet die Größenbegrenzung."}
                digest.update(block)
            final=os.fstat(stream.fileno())
            if (count!=opened.st_size or final.st_size!=opened.st_size or
                final.st_mtime_ns!=opened.st_mtime_ns or final.st_ctime_ns!=opened.st_ctime_ns or
                final.st_dev!=opened.st_dev or final.st_ino!=opened.st_ino):
                return {"error":"Datei wurde während der Untersuchung verändert."}
    except (OSError,ValueError) as exc:
        return {"error":"Firmwaredatei konnte nicht sicher gelesen werden: "+str(exc)}
    return {"timestamp":datetime.now(timezone.utc).isoformat(),
            "filename":file.name,"size_bytes":count,
            "sha256":digest.hexdigest(),"header_hex":preview.hex(" "),
            "controller_compatibility":"NICHT VERIFIZIERT",
            "signature_verified":False,"firmware_flashing_enabled":False,
            "notice":"Hash und Header sind keine Echtheits- oder Kompatibilitätsbestätigung. Kein Flash."}

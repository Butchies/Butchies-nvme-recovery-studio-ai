"""Offline signature scan of an existing regular image file.

Never opens physical drives or writes to source files. Findings are candidates,
not recovered files or proof of filesystem integrity.
"""
import os
import stat
import time

SIGNATURES=((b"\x89PNG\r\n\x1a\n","PNG"),(b"\xff\xd8\xff","JPEG"),
            (b"%PDF-","PDF"),(b"PK\x03\x04","ZIP"),(b"SQLite format 3\x00","SQLite"))
CHUNK=1024*1024
MAX_RESULTS=1000

def scan(path, progress=None, cancelled=None):
    result={"mode":"offline_image_signature_scan","source_writes_allowed":False,
            "physical_device_access":False,"scanned_bytes":0,"matches":[],
            "completed":False,"cancelled":False}
    try:
        info=os.lstat(path)
        if not stat.S_ISREG(info.st_mode) or stat.S_ISLNK(info.st_mode):
            return dict(result,error="Nur vorhandene reguläre Image-Dateien sind erlaubt.")
        if info.st_size<=0:
            return dict(result,error="Image-Datei ist leer.")
        result["size_bytes"]=info.st_size
        started=time.monotonic()
        overlap=max(len(sig) for sig,_ in SIGNATURES)-1
        previous=b""
        with open(path,"rb",buffering=0) as stream:
            if os.fstat(stream.fileno()).st_ino!=info.st_ino or os.fstat(stream.fileno()).st_dev!=info.st_dev:
                return dict(result,error="Dateiidentität beim Öffnen verändert.")
            while True:
                if cancelled and cancelled():
                    result["cancelled"]=True
                    break
                chunk=stream.read(CHUNK)
                if not chunk:
                    result["completed"]=True
                    break
                data=previous+chunk
                base=result["scanned_bytes"]-len(previous)
                for signature,name in SIGNATURES:
                    offset=0
                    while len(result["matches"])<MAX_RESULTS:
                        pos=data.find(signature,offset)
                        if pos<0: break
                        absolute=base+pos
                        if absolute>=0:
                            result["matches"].append({"offset_bytes":absolute,"signature":name})
                        offset=pos+1
                result["scanned_bytes"]+=len(chunk)
                previous=data[-overlap:]
                if progress:
                    progress(result["scanned_bytes"],info.st_size,len(result["matches"]))
                if len(result["matches"])>=MAX_RESULTS:
                    result["limit_reached"]=True
                    break
        result["elapsed_seconds"]=round(time.monotonic()-started,3)
        result["warning"]="Signaturtreffer können falsch positiv sein; keine Dateigrenzen oder Dateiintegrität geprüft. Kein Carving und keine Wiederherstellung."
    except OSError as exc:
        result["error"]=str(exc)
    return result

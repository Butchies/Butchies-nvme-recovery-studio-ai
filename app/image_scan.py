"""Offline signature scan of an existing regular image file.

Never opens physical drives or writes to source files. Findings are candidates,
not recovered files or proof of filesystem integrity.
"""
import os
import stat
import time
import png_inspection

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
        with os.fdopen(os.open(path,os.O_RDONLY|getattr(os,"O_CLOEXEC",0)|getattr(os,"O_NOFOLLOW",0)|getattr(os,"O_NONBLOCK",0)),"rb",buffering=0) as stream:
            opened=os.fstat(stream.fileno())
            if not stat.S_ISREG(opened.st_mode) or opened.st_ino!=info.st_ino or opened.st_dev!=info.st_dev or opened.st_size!=info.st_size:
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
                if progress and (result["scanned_bytes"]//(16*CHUNK)!=(result["scanned_bytes"]-len(chunk))//(16*CHUNK) or result["scanned_bytes"]>=info.st_size):
                    progress(result["scanned_bytes"],info.st_size,len(result["matches"]))
                if len(result["matches"])>=MAX_RESULTS:
                    result["limit_reached"]=True
                    break
        if result["matches"] and not result["cancelled"] and not (cancelled and cancelled()):
            # Reopen only the verified regular image, never a physical device.
            with os.fdopen(os.open(path,os.O_RDONLY|getattr(os,"O_CLOEXEC",0)|getattr(os,"O_NOFOLLOW",0)),"rb",buffering=0) as verify:
                again=os.fstat(verify.fileno())
                if stat.S_ISREG(again.st_mode) and again.st_ino==info.st_ino and again.st_dev==info.st_dev and again.st_size==info.st_size:
                    result["png_inspection"]=png_inspection.inspect_candidates(verify,result["matches"],info.st_size)
                else:
                    result["png_inspection_warning"]="Dateiidentität vor PNG-Prüfung verändert."
        result["elapsed_seconds"]=round(time.monotonic()-started,3)
        result["warning"]="Signaturtreffer können falsch positiv sein. Nur PNG-Kandidaten erhalten eine begrenzte Chunk-/CRC-Prüfung; kein Export und keine Wiederherstellung."
    except OSError as exc:
        result["error"]=str(exc)
    return result

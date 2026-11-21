"""Windows-only recovery of PNG/JPEG from an existing regular image file.

Never opens a physical disk. No source writes. Only exports into a pre-existing
directory on a different Windows volume, with exclusive file creation.
"""
import hashlib
import os
import stat
import jpeg_inspection
import png_inspection

CHUNK=1024*1024
LIMIT=32

def export_files(source,destination,findings,cancelled=None,progress=None):
    result={"mode":"windows_offline_file_recovery","source_writes_allowed":False,
            "physical_device_access":False,"exported":[],"skipped":[],"cancelled":False}
    if os.name!="nt":
        return dict(result,error="Dieses Exportmodul ist nur für Windows vorgesehen.")
    if not isinstance(findings,(list,tuple)):
        return dict(result,error="Ungültige Kandidatenliste.")
    try:
        src=os.lstat(source)
        dst=os.lstat(destination)
        if not stat.S_ISREG(src.st_mode) or stat.S_ISLNK(src.st_mode):
            return dict(result,error="Quelle muss eine reguläre Image-Datei sein.")
        if not stat.S_ISDIR(dst.st_mode) or stat.S_ISLNK(dst.st_mode):
            return dict(result,error="Ziel muss ein vorhandenes echtes Verzeichnis sein.")
        source_real=os.path.realpath(source)
        target_real=os.path.realpath(destination)
        if os.path.splitdrive(source_real)[0].casefold()==os.path.splitdrive(target_real)[0].casefold():
            return dict(result,error="Quelle und Ziel müssen auf getrennten Windows-Laufwerken liegen.")
        with open(source,"rb",buffering=0) as reader:
            opened=os.fstat(reader.fileno())
            if (opened.st_size!=src.st_size or opened.st_dev!=src.st_dev
                    or opened.st_ino!=src.st_ino):
                return dict(result,error="Quell-Image wurde ausgetauscht.")
            for item in findings[:LIMIT]:
                if cancelled and cancelled():
                    result["cancelled"]=True
                    break
                if not isinstance(item,dict) or item.get("format") not in ("PNG","JPEG"):
                    result["skipped"].append({"reason":"Unbekanntes Dateiformat."})
                    continue
                kind=item["format"]
                inspector=png_inspection if kind=="PNG" else jpeg_inspection
                offset=item.get("offset_bytes")
                length=item.get("length_bytes")
                max_size=png_inspection.MAX_PNG_BYTES if kind=="PNG" else jpeg_inspection.MAX_JPEG_BYTES
                if (item.get("validated") is not True or item.get("complete") is not True
                        or type(offset) is not int or type(length) is not int
                        or offset<0 or length<=0 or length>max_size or offset+length>opened.st_size):
                    result["skipped"].append({"reason":"Ungültiger Dateikandidat."})
                    continue
                fresh=inspector.inspect(reader,offset,opened.st_size,cancelled=cancelled)
                if not fresh.get("validated") or not fresh.get("complete") or fresh.get("length_bytes")!=length:
                    result["skipped"].append({"offset_bytes":offset,"reason":"Strukturprüfung fehlgeschlagen."})
                    continue
                filename=f"recovered_{kind.lower()}_{offset:016x}.{('png' if kind=='PNG' else 'jpg')}"
                full=os.path.join(target_real,filename)
                output=None
                try:
                    if os.path.realpath(destination)!=target_real:
                        raise OSError("Zielverzeichnis wurde verändert.")
                    output=open(full,"xb",buffering=0)
                    digest=hashlib.sha256()
                    copied=0
                    reader.seek(offset)
                    while copied<length:
                        if cancelled and cancelled():
                            result["cancelled"]=True
                            raise InterruptedError("Export abgebrochen.")
                        block=reader.read(min(CHUNK,length-copied))
                        if not block:raise OSError("Image unvollständig.")
                        output.write(block)
                        digest.update(block)
                        copied+=len(block)
                        if progress:progress(len(result["exported"]),copied,length)
                    output.flush()
                    os.fsync(output.fileno())
                    with open(full,"rb",buffering=0) as verify:
                        checked=hashlib.sha256()
                        while True:
                            block=verify.read(CHUNK)
                            if not block:break
                            checked.update(block)
                        if checked.digest()!=digest.digest():
                            raise OSError("SHA-256-Nachprüfung fehlgeschlagen.")
                    result["exported"].append({"filename":filename,"offset_bytes":offset,
                                               "bytes":copied,"sha256":digest.hexdigest()})
                except (OSError,InterruptedError) as exc:
                    result["skipped"].append({"offset_bytes":offset,"reason":str(exc)})
                    if output is not None:
                        output.close()
                        output=None
                        try:os.unlink(full)
                        except OSError:pass
                finally:
                    if output is not None:output.close()
                if result["cancelled"]:break
    except (OSError,TypeError,ValueError) as exc:
        result["error"]=str(exc)
    result["warning"]="Offline-Export aus regulären Images; keine vollständige Bilddekodierung."
    return result

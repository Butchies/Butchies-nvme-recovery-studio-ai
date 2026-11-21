"""Conservative PNG carving from an existing regular image to a new directory.

Never writes to a source image or block device. Only structurally validated,
contiguous PNG candidates may be copied. Existing files are never overwritten.
"""
import os
import stat
import hashlib
import png_inspection

COPY_CHUNK=1024*1024
MAX_EXPORTS=32

def export_pngs(image_path,destination_directory,findings,cancelled=None,progress=None):
    result={"mode":"offline_png_carving","source_writes_allowed":False,
            "physical_device_access":False,"exported":[],"skipped":[],"cancelled":False}
    if os.name!="posix" or not hasattr(os,"pread") or os.open not in os.supports_dir_fd:
        return dict(result,error="PNG-Export nur auf unterstützten POSIX-Systemen.")
    if not isinstance(findings,(list,tuple)):
        return dict(result,error="Ungültige Kandidatenliste.")
    source_fd=None
    try:
        source_info=os.lstat(image_path)
        if not stat.S_ISREG(source_info.st_mode) or stat.S_ISLNK(source_info.st_mode):
            return dict(result,error="Quelle muss eine reguläre Image-Datei sein.")
        target_info=os.lstat(destination_directory)
        if not stat.S_ISDIR(target_info.st_mode) or stat.S_ISLNK(target_info.st_mode):
            return dict(result,error="Ziel muss ein vorhandenes echtes Verzeichnis sein.")
        source_fd=os.open(image_path,os.O_RDONLY|getattr(os,"O_NOFOLLOW",0)|getattr(os,"O_CLOEXEC",0))
        opened=os.fstat(source_fd)
        if (not stat.S_ISREG(opened.st_mode) or opened.st_dev!=source_info.st_dev
                or opened.st_ino!=source_info.st_ino or opened.st_size!=source_info.st_size):
            return dict(result,error="Image-Identität hat sich verändert.")
        target_fd=os.open(destination_directory,os.O_RDONLY|getattr(os,"O_DIRECTORY",0)|getattr(os,"O_NOFOLLOW",0))
        try:
            target_opened=os.fstat(target_fd)
            if not stat.S_ISDIR(target_opened.st_mode) or target_opened.st_dev!=target_info.st_dev or target_opened.st_ino!=target_info.st_ino:
                return dict(result,error="Zielverzeichnis hat sich verändert.")
            # Conservative: refuse any output on the source filesystem.
            if opened.st_dev==target_opened.st_dev:
                return dict(result,error="Quelle und Ziel liegen auf demselben Dateisystem. Separates Ziel erforderlich.")
            with os.fdopen(os.dup(source_fd),"rb",buffering=0) as stream:
                for item in findings[:MAX_EXPORTS]:
                    if cancelled and cancelled():
                        result["cancelled"]=True
                        break
                    if not isinstance(item,dict) or item.get("format")!="PNG" or item.get("validated") is not True or item.get("complete") is not True:
                        result["skipped"].append({"reason":"Nicht vollständig validierter PNG-Kandidat."})
                        continue
                    offset=item.get("offset_bytes")
                    length=item.get("length_bytes")
                    if (type(offset) is not int or type(length) is not int or offset<0
                            or length<=0 or length>png_inspection.MAX_PNG_BYTES
                            or offset+length>opened.st_size):
                        result["skipped"].append({"reason":"Ungültige Kandidatenposition oder Länge."})
                        continue
                    fresh=png_inspection.inspect(stream,offset,opened.st_size,cancelled=cancelled)
                    if not fresh.get("validated") or not fresh.get("complete") or fresh.get("length_bytes")!=length:
                        result["skipped"].append({"offset_bytes":offset,"reason":"Erneute PNG-Prüfung fehlgeschlagen."})
                        continue
                    filename=f"recovered_png_{offset:016x}.png"
                    output_fd=None
                    created=False
                    try:
                        output_fd=os.open(filename,os.O_WRONLY|os.O_CREAT|os.O_EXCL|getattr(os,"O_NOFOLLOW",0),0o600,dir_fd=target_fd)
                        created=True
                        digest=hashlib.sha256()
                        copied=0
                        while copied<length:
                            if cancelled and cancelled():
                                result["cancelled"]=True
                                raise InterruptedError("Export abgebrochen.")
                            block=os.pread(source_fd,min(COPY_CHUNK,length-copied),offset+copied)
                            if not block:raise OSError("Image während Export abgeschnitten.")
                            written=0
                            while written<len(block):
                                n=os.write(output_fd,block[written:])
                                if n<=0:raise OSError("Zieldatei konnte nicht geschrieben werden.")
                                written+=n
                            digest.update(block)
                            copied+=len(block)
                            if progress and (copied==length or copied%(8*COPY_CHUNK)==0):
                                progress(len(result["exported"]),copied,length)
                        # Re-read the same source extent before accepting the export.
                        # This catches content changes even when the image size is stable.
                        source_check=hashlib.sha256()
                        source_checked=0
                        while source_checked<length:
                            if cancelled and cancelled():
                                result["cancelled"]=True
                                raise InterruptedError("Quellprüfung abgebrochen.")
                            block=os.pread(source_fd,min(COPY_CHUNK,length-source_checked),offset+source_checked)
                            if not block:
                                raise OSError("Quell-Image bei Nachprüfung abgeschnitten.")
                            source_check.update(block)
                            source_checked+=len(block)
                        if source_check.digest()!=digest.digest():
                            raise OSError("Quell-Image-Inhalt hat sich während des Exports verändert.")
                        os.fsync(output_fd)
                        # Verify the exact bytes persisted in the output file.
                        check_fd=os.open(filename,os.O_RDONLY|getattr(os,"O_NOFOLLOW",0),dir_fd=target_fd)
                        try:
                            check_info=os.fstat(check_fd)
                            owned_info=os.fstat(output_fd)
                            if (not stat.S_ISREG(check_info.st_mode) or check_info.st_dev!=owned_info.st_dev
                                    or check_info.st_ino!=owned_info.st_ino or check_info.st_size!=length):
                                raise OSError("Zieldatei nach Export verändert.")
                            checked=hashlib.sha256()
                            position=0
                            while position<length:
                                if cancelled and cancelled():
                                    result["cancelled"]=True
                                    raise InterruptedError("Prüfung abgebrochen.")
                                block=os.read(check_fd,min(COPY_CHUNK,length-position))
                                if not block:raise OSError("Zieldatei bei Nachprüfung unvollständig.")
                                checked.update(block)
                                position+=len(block)
                            if checked.digest()!=digest.digest():
                                raise OSError("SHA-256-Nachprüfung der Zieldatei fehlgeschlagen.")
                        finally:
                            os.close(check_fd)
                        if os.fstat(source_fd).st_size!=opened.st_size:
                            raise OSError("Quell-Image während des Exports verändert.")
                        result["exported"].append({"filename":filename,"offset_bytes":offset,
                                                   "bytes":copied,"sha256":digest.hexdigest()})
                    except (OSError,InterruptedError) as exc:
                        result["skipped"].append({"offset_bytes":offset,"reason":str(exc)})
                        if created and output_fd is not None:
                            try:
                                owned=os.fstat(output_fd)
                                current=os.stat(filename,dir_fd=target_fd,follow_symlinks=False)
                                if (stat.S_ISREG(current.st_mode) and owned.st_dev==current.st_dev
                                        and owned.st_ino==current.st_ino):
                                    os.unlink(filename,dir_fd=target_fd)
                            except OSError:pass
                    finally:
                        if output_fd is not None:os.close(output_fd)
                    if result["cancelled"]:break
        finally:
            os.close(target_fd)
    except (OSError,TypeError,ValueError) as exc:
        result["error"]=str(exc)
    finally:
        if source_fd is not None:os.close(source_fd)
    result["warning"]="Exportiert werden nur zusammenhängende PNG-Daten mit gültigen Chunk-CRC-Werten; Bilddekodierung und semantische Integrität sind nicht nachgewiesen."
    return result

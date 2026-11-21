"""Bounded, read-only PNG structure inspection within an existing image file.

This does not recover, export, or modify files. Only contiguous PNG candidates
are checked. CRC checks are performed on inspected chunks.
"""
import os
import struct
import zlib

PNG_SIGNATURE=b"\x89PNG\r\n\x1a\n"
MAX_PNG_BYTES=32*1024*1024
MAX_CHUNKS=4096
MAX_CANDIDATES=32

def inspect(stream,offset,file_size,cancelled=None):
    result={"offset_bytes":offset,"format":"PNG","validated":False,
            "complete":False,"crc_errors":0,"chunks_checked":0}
    if offset<0 or offset+8>file_size:
        return dict(result,error="Position außerhalb des Images.")
    stream.seek(offset)
    if stream.read(8)!=PNG_SIGNATURE:
        return dict(result,error="PNG-Signatur fehlt.")
    position=offset+8
    seen_ihdr=False
    seen_idat=False
    idat_closed=False
    seen_plte=False
    color_type=None
    for _ in range(MAX_CHUNKS):
        if cancelled and cancelled():
            return dict(result,cancelled=True,error="PNG-Prüfung abgebrochen.")
        if position-offset+12>MAX_PNG_BYTES or position+12>file_size:
            return dict(result,error="PNG-Ende außerhalb der Prüfgrenze oder des Images.")
        header=stream.read(8)
        if len(header)!=8:
            return dict(result,error="Unvollständiger PNG-Chunk-Header.")
        length=struct.unpack(">I",header[:4])[0]
        kind=header[4:]
        if length>MAX_PNG_BYTES or position-offset+12+length>MAX_PNG_BYTES or position+12+length>file_size:
            return dict(result,error="PNG-Chunk überschreitet Prüfgrenze.")
        if not all((65<=byte<=90 or 97<=byte<=122) for byte in kind):
            return dict(result,error="Ungültiger PNG-Chunk-Typ.")
        if not seen_ihdr and (kind!=b"IHDR" or length!=13):
            return dict(result,error="IHDR fehlt oder hat falsche Länge.")
        if kind==b"IHDR" and seen_ihdr:
            return dict(result,error="Mehrfacher IHDR-Block.")
        if kind==b"PLTE":
            if seen_plte or seen_idat or length==0 or length%3 or length>768:
                return dict(result,error="Ungültiger oder falsch positionierter PLTE-Block.")
            seen_plte=True
        if kind==b"IDAT":
            if idat_closed:
                return dict(result,error="Nicht zusammenhängende IDAT-Blöcke.")
            if color_type==3 and not seen_plte:
                return dict(result,error="Indizierte PNG-Farben ohne PLTE.")
        elif seen_idat:
            idat_closed=True
        crc=zlib.crc32(kind)
        remaining=length
        ihdr=b""
        while remaining:
            if cancelled and cancelled():
                return dict(result,cancelled=True,error="PNG-Prüfung abgebrochen.")
            block=stream.read(min(remaining,1024*1024))
            if not block:
                return dict(result,error="PNG-Chunk abgeschnitten.")
            if kind==b"IHDR":ihdr+=block
            crc=zlib.crc32(block,crc)
            remaining-=len(block)
        stored=stream.read(4)
        if len(stored)!=4:
            return dict(result,error="PNG-CRC abgeschnitten.")
        result["chunks_checked"]+=1
        if crc&0xffffffff!=struct.unpack(">I",stored)[0]:
            result["crc_errors"]+=1
        position+=12+length
        if kind==b"IHDR":
            width,height=struct.unpack(">II",ihdr[:8])
            if width==0 or height==0:
                return dict(result,error="Ungültige PNG-Bildabmessungen.")
            bit_depth=ihdr[8]
            color_type=ihdr[9]
            compression,filter_method,interlace=ihdr[10:13]
            allowed={0:(1,2,4,8,16),2:(8,16),3:(1,2,4,8),4:(8,16),6:(8,16)}
            if (color_type not in allowed or bit_depth not in allowed[color_type]
                    or compression!=0 or filter_method!=0 or interlace not in (0,1)):
                return dict(result,error="Ungültige PNG-IHDR-Kodierungsparameter.")
            result["bit_depth"]=bit_depth
            result["color_type"]=color_type
            result["width"]=width
            result["height"]=height
            seen_ihdr=True
        elif kind==b"IDAT":
            seen_idat=True
        elif kind==b"IEND":
            if length!=0 or not seen_idat:
                return dict(result,error="IEND ungültig oder Bilddaten fehlen.")
            result["complete"]=True
            result["length_bytes"]=position-offset
            result["validated"]=result["crc_errors"]==0
            result["note"]="PNG-Chunk-Struktur und CRC geprüft; Dekodierung und Bildinhalt nicht validiert."
            return result
    return dict(result,error="Maximale Anzahl PNG-Chunks erreicht.")

def inspect_candidates(stream,matches,file_size,cancelled=None):
    findings=[]
    for match in matches:
        if cancelled and cancelled():break
        if match.get("signature")=="PNG":
            if len(findings)>=MAX_CANDIDATES:break
            findings.append(inspect(stream,match["offset_bytes"],file_size,cancelled=cancelled))
    return findings

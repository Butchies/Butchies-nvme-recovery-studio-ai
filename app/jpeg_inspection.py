"""Offline JPEG candidate inspection for raw image recovery.

Checks marker framing and locates an end-of-image marker without writing files.
Not a decoder; entropy data can be corrupt despite a plausible marker sequence.
"""
import os

MAX_JPEG_BYTES=64*1024*1024
MAX_CANDIDATES=64
SOI=b"\xff\xd8"
EOI=b"\xff\xd9"

def inspect(stream,offset,file_size,cancelled=None):
    result={"format":"JPEG","offset_bytes":offset,"complete":False,"validated":False}
    if type(offset) is not int or offset<0 or offset+4>file_size:
        return dict(result,error="Position außerhalb des Images.")
    stream.seek(offset)
    if stream.read(2)!=SOI:
        return dict(result,error="JPEG-Startmarkierung fehlt.")
    position=offset+2
    saw_sof=False
    saw_sos=False
    in_entropy=False
    while position-offset<MAX_JPEG_BYTES and position+2<=file_size:
        if cancelled and cancelled():
            return dict(result,cancelled=True,error="JPEG-Prüfung abgebrochen.")
        marker=stream.read(1)
        position+=1
        if not marker:
            break
        if marker!=b"\xff":
            if in_entropy:
                continue
            return dict(result,error="Unerwartete Daten außerhalb des Bildstroms.")
        while position<file_size:
            code=stream.read(1)
            position+=1
            if code!=b"\xff":break
        else:break
        if code==b"\x00" and in_entropy:continue
        if code==b"\xd9":
            if not saw_sof or not saw_sos:
                return dict(result,error="JPEG ohne Bildrahmen oder Scan.")
            result.update(complete=True,validated=True,length_bytes=position-offset,
                          note="JPEG-Markerstruktur plausibel; keine Bilddekodierung.")
            return result
        value=code[0]
        if in_entropy and 0xd0<=value<=0xd7:continue
        in_entropy=False
        if value==0xd8 or value==0x01 or 0xd0<=value<=0xd7:
            return dict(result,error="Unerwartete JPEG-Markierung.")
        if position+2>file_size or position-offset+2>MAX_JPEG_BYTES:break
        size_bytes=stream.read(2)
        position+=2
        if len(size_bytes)!=2:break
        segment_length=int.from_bytes(size_bytes,"big")
        if segment_length<2 or position+segment_length-2>file_size or position+segment_length-2-offset>MAX_JPEG_BYTES:
            return dict(result,error="JPEG-Segmentlänge ungültig.")
        if value in (0xc0,0xc1,0xc2,0xc3,0xc5,0xc6,0xc7,0xc9,0xca,0xcb,0xcd,0xce,0xcf):
            if segment_length<8:return dict(result,error="JPEG-Bildrahmen zu kurz.")
            header=stream.read(5)
            if len(header)!=5:break
            height=int.from_bytes(header[1:3],"big")
            width=int.from_bytes(header[3:5],"big")
            if not width or not height:return dict(result,error="JPEG-Bildabmessungen ungültig.")
            result.update(width=width,height=height)
            saw_sof=True
            stream.seek(segment_length-7,os.SEEK_CUR)
        else:
            stream.seek(segment_length-2,os.SEEK_CUR)
        position+=segment_length-2
        if value==0xda:
            if not saw_sof:return dict(result,error="JPEG-Scan ohne Bildrahmen.")
            saw_sos=True
            in_entropy=True
    return dict(result,error="JPEG-Ende nicht innerhalb der Prüfgrenze gefunden.")

def inspect_candidates(stream,matches,file_size,cancelled=None):
    results=[]
    for item in matches:
        if cancelled and cancelled():break
        if item.get("signature")!="JPEG":continue
        if len(results)>=MAX_CANDIDATES:break
        results.append(inspect(stream,item.get("offset_bytes"),file_size,cancelled))
    return results

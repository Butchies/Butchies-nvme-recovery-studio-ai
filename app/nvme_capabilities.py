"""Interpret NVMe controller capabilities without sending device commands.

These are observations, not firmware compatibility or recovery authorization.
"""
def _integer(value):
    try:
        if isinstance(value,bool): return None
        if isinstance(value,int): return value if value>=0 else None
        if isinstance(value,str):
            value=value.strip()
            return int(value,16) if value.lower().startswith("0x") else int(value)
    except (TypeError,ValueError,OverflowError):
        pass
    return None

def inspect(diagnostics):
    if not isinstance(diagnostics,dict):
        return {"error":"NVMe-Diagnose fehlt","firmware_write_allowed":False}
    ctrl=diagnostics.get("identify")
    fw=diagnostics.get("firmware_slots")
    ns=diagnostics.get("namespace")
    result={"mode":"offline_read_only_interpretation","firmware_write_allowed":False,
            "firmware_compatible":False,"observations":[],"limitations":[]}
    if isinstance(ctrl,dict) and not ctrl.get("error"):
        frmw=_integer(ctrl.get("frmw"))
        oacs=_integer(ctrl.get("oacs"))
        if frmw is not None:
            count=(frmw>>1)&7
            result["advertised_firmware_slots"]=count
            result["slot_1_read_only"]=bool(frmw&1)
            result["activation_without_reset_advertised"]=bool(frmw&16)
            result["observations"].append("Controller-Firmware-Slot-Fähigkeiten aus Identify Controller dekodiert.")
        if oacs is not None:
            result["firmware_download_commit_advertised"]=bool(oacs&4)
            result["observations"].append("Firmware-Download/Commit-Fähigkeit ist ein Controller-Merkmal, keine Recovery-Freigabe.")
    else:
        result["limitations"].append("Identify Controller fehlt oder ist nicht lesbar.")
    if isinstance(fw,dict) and not fw.get("error"):
        afi=_integer(fw.get("afi"))
        if afi is not None:
            result["active_firmware_slot"]=afi&7
            next_slot=(afi>>4)&7
            result["next_reset_firmware_slot"]=next_slot if next_slot else None
        slots={}
        for i in range(1,8):
            key="frs"+str(i)
            if key in fw:
                value=fw[key]
                slots[str(i)]=str(value)[:128] if value is not None else None
        result["reported_slot_revisions"]=slots
    else:
        result["limitations"].append("Firmware-Slot-Log fehlt oder ist nicht lesbar.")
    if isinstance(ns,dict) and not ns.get("error"):
        for key in ("nsze","ncap","nuse","flbas","nlbaf"):
            if key in ns:
                val=_integer(ns[key])
                if val is not None: result[key]=val
        result["observations"].append("Namespace-Größe und Format-Metadaten aus Identify Namespace verfügbar.")
    else:
        result["limitations"].append("Identify Namespace fehlt oder ist nicht lesbar.")
    result["limitations"].append("Firmware-Slots, FRMW und OACS beweisen weder Firmware-Kompatibilität noch Vendor-Recovery-Zugriff.")
    return result

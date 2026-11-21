#!/usr/bin/env python3
"""Erzeugt die Controller- und Bridge-Vorlagen aus den offiziellen ID-Datenbanken.

Aufruf:  python3 tools/generate_catalog.py PFAD/pci.ids PFAD/usb.ids
Quellen: https://github.com/pciutils/pciids (pci.ids), https://github.com/usbids/usbids (usb.ids)
Lizenz der Daten: GPL-2.0-or-later oder BSD-3-Clause (siehe Kopf der Dateien).
Ausgabe: data/catalog/pci_nvme_ids.json, data/catalog/usb_storage_bridges.json
"""
import json, re, sys, pathlib

ROOT = pathlib.Path(__file__).resolve().parent.parent
OUT = ROOT / "data" / "catalog"
NVME = re.compile(r"nvm|ssd|solid state|non-volatile", re.I)
EXCLUDE = re.compile(r"modem|lte|dvb|tv|wireless|wlan|broadband|cinergy|host controller|nvmemprog|ethernet|network|audio|hub", re.I)


def parse(path):
    vendors, cur, dev, version = {}, None, None, ""
    for line in open(path, encoding="utf-8", errors="replace"):
        if line.startswith("#"):
            m = re.match(r"#\s*Version:\s*(\S+)", line)
            if m: version = m.group(1)
            continue
        if line.startswith("C ") or line.startswith("AT "):
            break
        if not line.strip():
            continue
        if not line.startswith("\t"):
            vid, name = line.strip().split(None, 1)
            cur = vendors[vid.lower()] = {"name": name.strip(), "devices": {}}
        elif not line.startswith("\t\t"):
            did, name = line.strip().split(None, 1)
            dev = cur["devices"][did.lower()] = {"name": name.strip(), "subsystems": {}}
        else:
            parts = line.strip().split(None, 2)
            if len(parts) == 3 and dev is not None:
                dev["subsystems"][f"{parts[0].lower()}:{parts[1].lower()}"] = parts[2].strip()
    return vendors, version


def main(pci_path, usb_path):
    pci, pci_ver = parse(pci_path)
    out = {"quelle": "pci.ids (pciutils/pciids)", "version": pci_ver,
           "lizenz": "GPL-2.0-or-later oder BSD-3-Clause",
           "hinweis": "Alle Geräte mit NVMe/SSD-Bezug; Subsysteme ordnen Endkundenprodukte dem Controller zu.",
           "vendors": {}}
    for vid, v in pci.items():
        devs = {did: d for did, d in v["devices"].items()
                if NVME.search(d["name"]) or any(NVME.search(s) for s in d["subsystems"].values())}
        if devs:
            out["vendors"][vid] = {"name": v["name"], "devices": devs}
    (OUT / "pci_nvme_ids.json").write_text(json.dumps(out, ensure_ascii=False, indent=1, sort_keys=True), encoding="utf-8")

    usb, usb_ver = parse(usb_path)
    bridges = {"quelle": "usb.ids (usbids/usbids)", "version": usb_ver,
               "lizenz": "GPL-2.0-or-later oder BSD-3-Clause", "devices": {}}
    for vid, v in usb.items():
        for did, d in v["devices"].items():
            name = d["name"]
            if re.search(r"sata|nvme|ssd|bridge|enclosure|ata/atapi", name, re.I) and not EXCLUDE.search(name):
                kind = "nvme" if re.search(r"nvme|pcie|m\.2", name, re.I) else ("sata" if re.search(r"sata|ata/atapi", name, re.I) else "unbekannt")
                bridges["devices"][f"{vid}:{did}"] = {"vendor": v["name"], "name": name, "typ": kind}
    (OUT / "usb_storage_bridges.json").write_text(json.dumps(bridges, ensure_ascii=False, indent=1, sort_keys=True), encoding="utf-8")
    nd = sum(len(v["devices"]) for v in out["vendors"].values())
    print(f"pci.ids {pci_ver}: {len(out['vendors'])} Hersteller, {nd} NVMe/SSD-Geräte")
    print(f"usb.ids {usb_ver}: {len(bridges['devices'])} USB-Speicher-/Bridge-Einträge")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])

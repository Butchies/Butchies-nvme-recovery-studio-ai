# Vorlagen für die Recovery-KI

| Datei | Inhalt | Herkunft |
|---|---|---|
| `pci_nvme_ids.json` | Alle Hersteller und Geräte mit NVMe-/SSD-Bezug inkl. Subsystemen (Endkundenprodukte → Controller) | erzeugt aus pci.ids (pciutils), GPL-2.0+/BSD-3 |
| `usb_storage_bridges.json` | USB-Speicher- und Bridge-Geräte (SATA/NVMe) | erzeugt aus usb.ids, GPL-2.0+/BSD-3 |
| `usb_nvme_bridges.json` | USB-NVMe-Bridgechips und was sie durchreichen | Fachwissen, mit Vertrauensangabe |
| `controller_families.json` | Controllerfamilien je Hersteller: Notmodus, Reparaturwege, Werkzeuge, Datenverlust, Fehlerbilder | Fachwissen, mit Vertrauensangabe |
| `tools.json` | Werkzeuge der KI mit offiziellen Quellen, Erkennung und Nutzung (Liste: `docs/WERKZEUGE_DOWNLOADLISTE_DE.md`) | recherchiert |
| `firmware_issues.json` | Bekannte fehlerhafte Firmwarestände und offizielle Korrekturen | recherchiert, mit Belegen |

Die Dateien werden in das Programm eingebettet. Eine gleichnamige Datei in `%APPDATA%/NVMeRecoveryStudioAI/vorlagen/` hat Vorrang (Aktualisierung ohne Neubau).

Aktualisieren der ID-Daten:

```
git clone --depth 1 https://github.com/pciutils/pciids
git clone --depth 1 https://github.com/usbids/usbids
python3 tools/generate_catalog.py pciids/pci.ids usbids/usb.ids
```

Herstellerwerkzeuge (MPTools) und Firmwaredateien sind nicht enthalten: Sie sind nicht frei verteilbar bzw. werden nur aus der offiziellen Quelle bezogen.
Neue, unbekannte Controller legt die KI als Vorschlag in Quarantäne ab (`vorlagenVorschlaege` in `ki-wissen.json`); daraus entsteht nie automatisch eine Schreibfreigabe.

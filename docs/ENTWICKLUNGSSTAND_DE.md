# Entwicklungsstand

## Erste Codebasis
- `core/device_inventory.py`: Linux-sysfs-basierte, ausschließlich lesende NVMe-Controllererkennung und JSON-Ausgabe.
- `apps/linux/inventory_cli.py`: Kommandozeilen-Einstieg für die Erkennung.
- `core/approval_policy.py`: konservative Planungsprüfung für Firmware-Schreibvorschläge, **kein Flash-Executor**.
- `ai/case_memory.py`: lokale SQLite-Fallakten mit zeitgestempelten Ereignissen, Evidenz und Erklärungen.

## Noch nicht implementiert
Windows-USB-Erkennung, Qt-Oberfläche, WLAN-Kiosk, bootfähiges ISO, lokales Sprachmodell, echte NVMe-SMART- und Fehlerlogauswertung, Firmwarebibliothek, Firmwaredownload/-flash, KI-generierte Controller-Firmware, isolierte Codeausführung, Live-UI.

## Grenzen
Die reine sysfs-Erkennung ersetzt keine vollständige Hardwareidentifikation. PCIe-Attribute können je nach Kernel/Treiber fehlen. Seriennummern in exportierten JSON-Dateien sind sensible Gerätekennungen; nur lokal verwenden.

## Entwicklungsregel
Die Implementierung erfolgt schrittweise. Ein vorhandenes Modul ist kein Beleg für Hardwaretauglichkeit. Keine Reparatur oder Schreiboperation ohne explizite, gerätegebundene Freigabe.

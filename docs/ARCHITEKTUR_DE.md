# Technische Architektur

## Komponenten
- `apps/windows/`: USB-only Desktop-App, C++20 und Qt 6/QML.
- `apps/linux/`: Linux Recovery UI und Boot-Konfiguration.
- `core/device-discovery/`: Geräteinventar und eindeutige Hardwareidentität.
- `core/diagnostics/`: NVMe/PCIe/SMART/USB-Bridge-Leseoperationen.
- `core/firmware/`: signierter Firmwarekatalog, Kompatibilitäts- und Flashplanung.
- `core/recovery/`: herstellerspezifische, ausdrücklich freigegebene Recovery-Adapter.
- `ai/`: lokales Modell, Werkzeugsteuerung, evidenzbasierte Planung, Wissensspeicher.
- `knowledge/`: Controllerprofile, Bridgeprofile, Fehlercodes, Recovery-Fälle.
- `docs/`: Offline-Hilfe und Entwicklungsspezifikation.
- `packaging/`: Windows-Installer und Linux-UEFI-Image-Build.

## Ausführungsmodell
1. Geräte entdecken und unveränderliche Geräteidentität erfassen.
2. Bus/Topologie prüfen: Windows nur USB; Linux direkte PCIe-Ziele erlaubt.
3. Lesende Diagnosen ausführen, Rohdaten und Interpretationen getrennt speichern.
4. KI erstellt begründete Hypothesen und priorisierte Reparaturpläne.
5. Firmware- und Gerätekompatibilität deterministisch prüfen, unabhängig von der KI.
6. Risikoprüfung und explizite Nutzerfreigabe für konkrete Zielgeräte und Aktionen.
7. Privilegierter Executor führt nur freigegebene, allowlist-basierte Operationen aus.
8. Ergebnis erneut auslesen, mit Vorzustand vergleichen, Fallakte archivieren.

## Offline First
Lokale Controller-/Bridge-Datenbanken, Hilfetexte, signierte erlaubte Firmwarepakete, Diagnoseprogramme und KI-Modell im Installationspaket bzw. auf USB. Fehlende proprietäre Images nicht vortäuschen. Online-Updates über HTTPS und verifizierte Metadaten.

## Datenspeicherung
SQLite für Geräteprofile, Fallakten, Provenienz, Reparaturhistorie und KI-Lernnotizen; Hashes für Firmware und Artefakte. Sensible Seriennummern und Logs lokal halten, keine automatische Cloudübertragung.

## Grenzen
USB-Bridges können NVMe-Admin- und Vendor-Befehle filtern. Direkte PCIe-Verbindung bietet mehr Diagnosezugriff, ersetzt aber keinen proprietären Bootloader oder Herstellerschlüssel.

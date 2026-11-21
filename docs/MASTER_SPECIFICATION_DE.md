# Master-Spezifikation – NVMe Recovery Studio AI

## Ziel
Ein herstellerübergreifendes Diagnose- und Recovery-System für M.2-NVMe-SSDs, das defekte oder falsch geflashte Controller soweit technisch möglich wieder ansprechbar macht.

## Windows Edition
- Windows 10 und 11, C++20, Qt 6/QML.
- **USB-only:** nur nachweislich extern über USB verbundene NVMe-SSDs. Interne SATA/NVMe/PCIe-Laufwerke werden vollständig von Zielauswahl und Befehlsausführung ausgeschlossen.
- USB-Gerätetopologie, Bridge-Vendor/Product-ID, Bridge-Firmware, SCSI/UASP-Passthrough und unterstützte NVMe-Befehle prüfen.
- Bei unbekannter Zuordnung Schreibzugriffe blockieren.

## Linux Boot Edition
- Bootfähiger UEFI-USB-Stick mit schlankem Linux und direktem Start der Vollbild-Recovery-Anwendung, ohne normalen Desktop.
- Direkte PCIe/NVMe-Erkennung über sysfs, libnvme, nvme-cli, pciutils und smartmontools.
- WLAN-Einrichtung über NetworkManager; HTTPS-Downloads, Updates, Offline-Betrieb.
- Eigene persistente, möglichst verschlüsselte Datenpartition für Modelle, Fallakten, Datenbanken und Pakete.
- Recovery-USB-Stick selbst niemals als Flashziel anbieten.
- Alle internen Datenträger standardmäßig schreibgesperrt; ausschließlich ausdrücklich gewähltes Reparaturziel nach Prüfung freigeben.

## Rechner- und Host-Hardwareanalyse
- CPU, RAM, Mainboard, BIOS/UEFI, Chipsatz, USB-/PCIe-Controller und relevante Treiberinformationen soweit verfügbar erfassen.
- Verbindungstopologie, Linkgeschwindigkeit, Systemereignisse und Temperaturhinweise in die KI-Einordnung einbeziehen.
- Host-, Bridge- und SSD-Probleme in der Diagnose voneinander abgrenzen.
- Rechneranalyse bleibt lesend; interne Windows-Laufwerke werden dadurch nicht zu Recovery-Zielen.

## Diagnose
- PCI Vendor/Device/Subvendor/Subdevice IDs, Controller-/Namespace-Identify, Firmware-Revision und Slots.
- SMART/Health, Temperatur, Media/Data Integrity Errors, Error Information Log, NVMe Statuscodes.
- PCIe-Linkzustand, Treiberbindung, Reset-/Enumeration-Probleme.
- Fehlerursachen als Hypothesen mit Evidenz und Unsicherheitsgrad darstellen.
- Bei Controller ohne PCIe-Antwort Hardwaremessungen/Debug-Adapter als Voraussetzung dokumentieren, keine unmögliche Software-Reparatur behaupten.

## Firmware Recovery
- Original-Firmwarekatalog nach Controller, Board, NAND, Revision, SSD-OEM und Signatur.
- Standard-Firmware-Download/Commit, alternative Firmware-Slots und dokumentierte Bootloader-Recovery-Verfahren.
- Recovery-Versuche kontrolliert und begrenzt; nach jedem Schritt erneute Identifikation und Ergebnisprüfung.
- Keine universelle Firmware und keine Zusage, dass proprietäre Controller ohne Herstellerzugang flashbar sind.

## Experimenteller KI-Custom-Firmware-Modus
- Wenn keine passende Original-Firmware existiert: Architektur und Bootpfad recherchieren, ggf. Diagnose-, Boot- oder vollständige Firmware entwickeln.
- Generierten Quellcode und Binärartefakte nachvollziehbar versionieren, statisch prüfen, bauen und soweit möglich isoliert simulieren.
- Nur auf dokumentierter, programmierbarer und hinreichend kompatibler Hardware; Signatur- und Bootloader-Sperren respektieren.
- **Separate Nutzerfreigabe vor jedem tatsächlichen Schreibversuch** mit Zielidentität, erwarteten Änderungen, Risiken und Datenverlustwarnung.
- Eine erfolgreiche Simulation ist kein Nachweis für echte Hardwarekompatibilität.

## KI und Lernen
- Lokales Modell für Offline-Analyse; optionale Online-Unterstützung.
- Fallgedächtnis mit Hardware-Fingerprints, Befunden, Hypothesen, Aktionen und verifizierten Resultaten.
- Neue Diagnosevorschläge und Profile in Quarantäne bis zur Validierung; keine selbsttätige Erweiterung von privilegierten Flashrechten.
- Aus Erfolgen und Fehlschlägen lernen, nachvollziehbar und rücknehmbar.

## Live-Monitor
- Benutzer kann Live-Messwerte, Schritte, Fehlercodes, Evidenz, Entscheidungserklärungen, Quellen und Resultate sehen.
- Bei KI-Codegenerierung: Quelltext, Diff, Build- und Prüfprotokolle.
- Exportierbare, lokal gespeicherte Fallakten.
- Keine Behauptung, verborgene interne Modellgedanken auszugeben; stattdessen explizite nachvollziehbare Entscheidungsprotokolle.

## UI
- Dunkle professionelle Oberfläche mit Dashboard, Geräteauswahl, Diagnose, Firmware Manager, Recovery, KI-Live-Monitor, Hilfe, WLAN, Protokollen und Einstellungen.
- Demo-/Simulationsmodus klar kennzeichnen; keine simulierten Ergebnisse als echte Messung darstellen.

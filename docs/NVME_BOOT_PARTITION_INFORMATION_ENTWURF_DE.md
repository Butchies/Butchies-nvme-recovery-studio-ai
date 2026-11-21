# NVMe-Boot-Partition-Informationen – Produktentwurf

## Ziel
Die Anwendung soll optionale NVMe-Boot-Partitionen von gewöhnlichen Nutzerdaten-Partitionen und vom Linux-Boot-USB-Stick unterscheiden.

## Geräteübersicht
- Unterstützung von NVMe-Boot-Partitionen nur anzeigen, wenn sie tatsächlich bestätigt ist.
- Vorhandene Boot-Partition-Informationen, Größen und Zugriffsgrenzen dokumentieren, soweit verfügbar.
- Die Begriffe NVMe-Boot-Partition, EFI-Systempartition und bootfähiger USB-Stick klar auseinanderhalten.
- Möglichen Schreibschutz oder Sperrzustand mit Datenquelle darstellen.
- Unbekannte Herstellerdetails nicht als frei beschreibbar einstufen.

## KI-Erklärung
- Boot-Partitionen als spezielle Controller-Funktion erläutern.
- Grenzen bei fehlendem PCIe-Zugriff oder durch USB-Bridge blockierten Informationen nennen.
- Keine Annahme treffen, dass eine beschädigte Betriebssysteminstallation durch Änderungen an NVMe-Boot-Partitionen repariert wird.
- Firmware- und Boot-Partition-Maßnahmen strikt von Datenrettung trennen.

## Schutz
- Keine automatischen Boot-Partition-Schreibvorgänge.
- Keine Entsperr- oder Schutzänderungen aus der normalen Diagnose heraus.
- Vorhandene Bootinformationen nicht mit dem Rettungsziel oder dem Linux-Systemstick verwechseln.

## Noch gewünschte Funktionen
Boot-Partition-Steckbrief, Begriffsabgrenzung, Schutzstatus und nachvollziehbare KI-Hinweise.

## Quelle
NVM Express Base Specification – Boot Partition:
https://nvmexpress.org/specification/nvm-express-base-specification/

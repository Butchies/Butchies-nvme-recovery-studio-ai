# Unbekannte SSDs und Controlleridentifikation – Produktentwurf

## Ziel
Wenn eine NVMe-SSD keinen vollständigen Modellnamen liefert oder nicht normal erkannt wird, soll die Anwendung alle tatsächlich verfügbaren Hinweise zusammenführen und den Benutzer durch die Identifikation führen.

## Mögliche Informationsquellen
- USB-Bridge-Kennung und durchgereichte NVMe-Informationen.
- PCI Vendor/Device IDs und Subsystemkennungen bei direktem PCIe-Zugriff.
- Modell, Firmware-Revision, Seriennummer und Namespace-Informationen, soweit auslesbar.
- Aufdrucke auf SSD-Platine, Controller und NAND-Chips, die der Benutzer selbst eingeben kann.
- Board-Revision, PCB-Kennung und Fotos als optionale Dokumentation.
- Bekannte Controllerprofile und Fallakten.

## Identifikationsansicht
- Erfasste Kennungen mit Herkunft anzeigen.
- Mehrere mögliche Controllerfamilien getrennt darstellen.
- Hinweise „bestätigt“, „wahrscheinlich“ und „unbekannt“ statt erfundener Sicherheit.
- Benutzer kann manuell Informationen ergänzen und korrigieren.
- Unterschiede zwischen Marketing-SSD-Modell und tatsächlich verbautem Controller erklären.

## KI-Unterstützung
- Vorhandene Kennungen mit lokalem Controllerwissen abgleichen.
- Alternative Hardwarevarianten desselben SSD-Modells berücksichtigen.
- Fehlende Angaben benennen, die für eine sichere Firmwarezuordnung wichtig wären.
- Bei vollständig fehlender Controllerantwort Grenzen der Softwareidentifikation erklären.
- Keine automatische Firmwareauswahl aus bloßen Vermutungen.

## Fallakten
- Manuelle Beobachtungen und Identifikationsänderungen dokumentieren.
- Quellen und Fotos nur nach Benutzerentscheidung speichern.
- Ergebnisse in Diagnose, Firmwarebibliothek und Recovery-Entscheidungshilfe übernehmen.

## Noch gewünschte Funktionen
Assistent für unbekannte SSDs, manuelle Hardwareangaben, Quellenvergleich und KI-Identifikationsansicht.

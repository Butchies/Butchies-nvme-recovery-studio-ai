# USB-Bridge-Fähigkeiten und Befehlsgrenzen – Produktentwurf

## Ziel
Die Anwendung soll transparent erklären, welche Informationen und Funktionen bei einer extern angeschlossenen NVMe-SSD durch die jeweilige USB-Bridge verfügbar sind. Eine sichtbare SSD ist nicht automatisch vollständig per NVMe steuerbar.

## Verbindungsprofil
- Hersteller, Produktkennung, Bridge-Chip und Firmwarestand anzeigen, soweit bekannt.
- USB-Protokoll, Anschlussgeschwindigkeit und tatsächlichen Verbindungsweg sichtbar machen.
- Erkennen, ob die SSD als USB-Massenspeicher erscheint und welche Informationen die Bridge durchreicht.
- Identität von USB-Gehäuse und darin eingebauter SSD getrennt behandeln.
- Fehlende Merkmale nicht als „SSD defekt“ interpretieren.

## Fähigkeitsübersicht
Je Funktion einen Status zeigen: verfügbar, durch Bridge nicht verfügbar, unbekannt oder nicht unterstützt.
- Grundlegendes Lesen und Geräteerkennung.
- NVMe Identify und Gesundheitsinformationen.
- Fehlerprotokolle und zusätzliche Diagnosedaten.
- Controller- und NAND-Identifikation.
- Firmwareinformationen und dokumentierte Recovery-Funktionen.
- Herstellerabhängige Spezialfunktionen.

## Benutzerführung
- Bei fehlender Durchleitung erklären, welcher Teil der Verbindung die Grenze darstellt.
- Hinweise auf andere Gehäuse oder direkten PCIe-Zugriff nur als Möglichkeiten darstellen, nicht als Erfolgsgarantie.
- Windows 10/11: interne SSDs weiterhin niemals als Recovery-Ziel verwenden.
- Linux-Bootedition: direkten PCIe-Zugriff als separate Anschlussart darstellen.
- Keine gefährlichen oder unbekannten Spezialbefehle allein zur Fähigkeitsfeststellung ausführen.

## KI-Auswertung
- Befunde aus SSD und USB-Bridge klar auseinanderhalten.
- Empfehlungen mit tatsächlich vorhandenen Befehlsmöglichkeiten abgleichen.
- Bei nicht verfügbarer Firmwarekommunikation keine scheinbare Flash-Fähigkeit behaupten.
- Verbindungseinschränkungen in Diagnosebericht und Fallakte aufnehmen.

## Noch gewünschte Funktionen
Bridge-Steckbrief, Fähigkeitsmatrix, verständliche Grenzen und alternative Anschlussberatung.

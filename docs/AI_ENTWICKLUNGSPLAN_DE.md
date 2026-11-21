# NVMe Recovery Studio AI – Entwurf und offene Programmierarbeiten

Dieses Dokument ist die Übergabe unseres gemeinsam festgelegten Entwurfs an eine andere KI. Es legt **keinen Testplan, keine Prüfverfahren und keine Arbeitsmethodik** fest. Wie die andere KI programmiert, verbessert und prüft, entscheidet sie selbst.

## Unsere Produktvision
NVMe Recovery Studio AI soll möglichst viele Fehler von M.2-NVMe-SSDs erkennen und technisch mögliche Reparaturen unterstützen. Das Programm soll ohne ständige Internetverbindung funktionieren und eine transparente, lernfähige KI integrieren. Nicht jede SSD lässt sich softwareseitig wiederherstellen.

## Windows-Edition
- Eigenständige Windows-10/11-Anwendung.
- Nur externe USB-NVMe-SSDs; keine internen SSDs als Ziel.
- USB-Bridge-Erkennung, verständliche Einschränkungen und passende Adapterhinweise.
- Diagnose, Firmwareverwaltung, KI-Assistent, Recovery und Live-Monitor.

## Linux-USB-Edition
- Bootfähiges USB-System, das direkt in die Recovery-Oberfläche startet, ohne normalen Desktop.
- Direkter PCIe-Zugriff auf interne NVMe-SSDs.
- WLAN, Online-Aktualisierung und weitgehend vollständiger Offlinebetrieb.
- Lokales KI-Modell, Controllerdatenbank, Firmwarebibliothek, Hilfedaten und persistenter Speicher.
- Boot-USB-Stick und andere Datenträger schützen.

## KI und Lernfähigkeit
- Controller und Fehlerbilder erkennen und erklären.
- Firmwarekompatibilität berücksichtigen.
- Eigene Fallakten und Wissensdatenbank mit Erkenntnissen aus Reparaturen aufbauen.
- Neue Diagnoseverfahren und Lösungswege vorschlagen und vorhandene verbessern.
- Fehlende Original-Firmware kann Anlass für experimentelle KI-Entwicklung eigener Diagnose-, Recovery- oder vollständiger Controller-Firmware sein.
- Technische Grenzen wie unbekannte Hardware, gesperrte Bootloader und Herstellersignaturen berücksichtigen.
- Vor tatsächlichen Schreib-/Flashaktionen ausdrückliche Benutzerfreigabe einholen.

## KI-Live-Monitor
- Live-Ansicht für Messwerte, Diagnoseschritte, Hypothesen, Begründungen und vorgeschlagene Aktionen.
- Einfache und detaillierte Expertenansicht.
- Quellcode, Änderungen und Fortschritt eigener Firmwareentwicklung anzeigen.
- Lokale Protokolle und Fallakten auch offline verfügbar halten.
- Sichtbar sind nachvollziehbare Analyseberichte, nicht die verborgenen internen Gedanken des KI-Modells.

## Firmware und Offline-Pakete
- Große lokale Sammlung rechtmäßig verteilbarer Firmware, Controllerprofile und Wiederherstellungsinformationen.
- Herstellerübergreifende Erkennung (u. a. Phison, Silicon Motion, Realtek, Innogrit, Maxio, Samsung, WD/SanDisk, Kioxia).
- Original-Firmware nur mit gesicherter Herkunft und passender Gerätezuordnung.
- Internet nur für fehlende Ressourcen, Updates und optionale Zusatzdienste benötigen.
- Zwei fertige Ausgabepakete: Windows-Installer und bootfähige Linux-USB-Edition.

## Gewünschte Oberfläche
Dashboard, Geräteliste, Diagnose, KI-Live-Monitor, Firmware-Manager, Recovery-Bereich, Reparaturhistorie, WLAN und Einstellungen. Modernes, dunkles, gut lesbares Design.

## Was noch programmiert werden muss
1. Vollständige Windows-USB-Geräteerkennung.
2. Erweiterte Linux-PCIe-/NVMe-Diagnose.
3. Gemeinsame Datenmodelle und Controllerprofile.
4. Moderne Windows- und Linux-Oberfläche.
5. KI-Live-Monitor mit realen Daten.
6. Lokales KI-Modell und lernfähige Wissensdatenbank.
7. Firmwarekatalog, Download und Kompatibilitätszuordnung.
8. Recovery-Funktionen mit Freigabedialog.
9. Experimenteller Modus für selbst entwickelte Controller-Firmware.
10. Linux-Bootmedium mit WLAN und Offline-Ressourcen.
11. Windows-Installationspaket und Aktualisierungsfunktionen.
12. Nach Fertigstellung: bebilderte PDF-Handbücher auf Deutsch und Englisch.

## Bisher vorhandene Projektdateien
- Dokumentierte Anforderungen und Architektur.
- Erste Linux-NVMe-Inventarisierung in Python.
- Erste Planungslogik für Schreibfreigaben.
- Lokale SQLite-Fallakten als Grundlage.

Diese Dateien sind nur ein Anfang, keine fertige Anwendung.

## Verantwortung der weiterentwickelnden KI
Die andere KI übernimmt die technische Planung, konkrete Programmierung, Optimierung, notwendige Prüfungen und Umsetzung. Diese Übergabe beschreibt ausschließlich, **was das Produkt können soll und welche Arbeiten noch fehlen**.

# NVMe Recovery Studio AI

KI-gesteuerte Diagnose, Datenrettung und Reparatur von M.2-NVMe-SSDs – als Windows-Programm für externe USB-SSDs und als Linux-Boot-Stick mit direktem PCIe-Zugriff.

**Projektstatus:** Funktionen programmiert, **noch nicht gebaut und nicht auf echter Hardware getestet**. Die „KI“ ist eine regelbasierte, lernende Entscheidungslogik mit Wissensbasis und Controller-Vorlagen – kein Sprachmodell.


## Zwei Editionen, ein Programm (`native/`, C++20, Qt 6, QML)
| | Windows 10/11 | Linux-Boot-Edition (USB-Stick) |
|---|---|---|
| Laufwerke | nur externe USB-SSDs, interne gesperrt | interne M.2-SSDs direkt über PCIe; Boot-Stick und Systemlaufwerke gesperrt |
| NVMe-Daten | über Bridge-Befehle mit smartctl | direkt (Identify, SMART, Fehlerprotokoll, Firmware-Slots) |
| Notmodus-Erkennung | „Bridge ohne Medium“ | PCIe-Gerät eines SSD-Controllerherstellers ohne NVMe-Funktion |
| Rettungsabbild | ja | ja (gleiches Format, Fortsetzen über Editionen hinweg) |
| Firmware-Manager | gesperrt (USB) | Standard-NVMe-Update mit Prüfungen, Freigabecode, Kontrolle |
| Boot-Stick erstellen | ja | `linux-boot/make-stick.sh` |

## Was die KI macht
1. Erkennt angeschlossene SSDs automatisch (Überwachung ab Programmstart).
2. Diagnostiziert selbstständig: Controller/Bridge laut Vorlagen (121 Hersteller, 835 NVMe-Controller/-Produkte aus pci.ids), SMART, Fehlerprotokoll, bekannte Firmwarefehler, Verlauf seit der letzten Untersuchung.
3. Bewertet Reparaturwege je Controllerfamilie (Datenrettung, Firmware-Update, Notmodus-Werkzeug, Labor) inkl. Datenverlust-Risiko.
4. Startet auf Knopfdruck die automatische Rettung (Mehrpass-Abbild, Lesekarte, Fortsetzen, Prüfsumme, Bericht).
5. Lernt aus jedem Versuch (lokale Wissensbasis je Gerät, USB-Bridge und Controller) und passt künftige Pläne an.
6. Zeigt alles im KI-Live-Monitor; Werkzeuge (smartmontools, Herstellertools, eigene MPTools) werden erkannt bzw. installiert.

## Sicherheitsgrundsätze
- Die defekte SSD wird für Diagnose und Rettung nur gelesen.
- Firmware schreiben nur im Firmware-Manager nach bestandenen Pflichtprüfungen, Datensicherung (oder ausdrücklichem Verzicht) und Eingabe des Bestätigungscodes.
- Notmodus-Werkzeuge setzen die SSD zurück (Datenverlust) und werden nur nach ausdrücklicher Freigabe eingesetzt.
- Keine Firmware oder Herstellerwerkzeuge aus inoffiziellen Quellen.

## Verzeichnisse
- `native/` – Anwendung (Windows und Linux)
- `data/catalog/` – Vorlagen: Controller, Bridges, Firmwarefehler, Werkzeuge (`README_DE.md`)
- `linux-boot/` – Bau des Boot-Sticks (`README_DE.md`)
- `docs/WERKZEUGE_DOWNLOADLISTE_DE.md` – offizielle Werkzeugquellen
- `app/` – ältere Python/Tkinter-Grundlage (Offline-Image-Analyse, PNG/JPEG-Export)

Weitere Dokumente: [Master-Spezifikation](docs/MASTER_SPECIFICATION_DE.md), [Sicherheit](docs/SICHERHEIT_DE.md), [Roadmap](docs/ROADMAP_DE.md), [Änderungsprotokoll](docs/CHANGELOG_DE.md).

## Bauen
- Windows: Qt 6.5+ (MSVC), `cmake -S native -B build && cmake --build build --config Release`
- Linux-Boot-Stick: siehe `linux-boot/README_DE.md`

## Rechtliches
Vorlagen aus pci.ids/usb.ids (GPL-2.0+/BSD-3). Proprietäre Firmware und Werkzeuge werden nicht mitgeliefert.

# NVMe Recovery Studio AI

KI-gestützte NVMe-Controllerdiagnose und Firmware-Wiederherstellung. **Projektstatus: erste ausführbare Desktop-Grundlage (0.1.0), noch keine funktionsfähige SSD-Recovery.**\n\n## Programmdateien\n- `app/main.py`: grafische, nur lesende Laufwerksinventarisierung und lokaler Fallaktenexport.\n- `.github/workflows/windows-installer.yml`: automatisierter Windows-Installer-Build.\n- `NVMeRecoveryStudioAI.iss`: Inno-Setup-Installationspaket.\n- `README_BUILD.md`: Build- und Funktionsstand.\n\n**Noch nicht fertig:** KI-Diagnose, SMART-/Controllerdiagnose, Datenrettung, Firmware-Flash und Linux-Bootimage.

## Zwei Editionen
- **Windows 10 und 11:** Ausschließlich externe NVMe-SSDs über USB. Interne Laufwerke sind weder Diagnose- noch Flash-Ziele.
- **Linux Boot Edition:** Eigenständiges UEFI-USB-Recovery-System, das direkt in die Vollbildanwendung startet. Direkte PCIe/NVMe-Diagnose interner Ziel-SSDs, WLAN, lokales KI-Modell und Offline-Werkzeuge.

## Geplante Module
- PCIe/NVMe/USB-Bridge-Erkennung und Kompatibilitätsassistent
- SMART, Error Logs, Identify, Firmware-Slots und Controllerdiagnose
- Firmwarekatalog mit Herkunft, Hash, Signatur, NAND-/Board-/Revision-Kompatibilität
- KI-Diagnose, Reparaturplanung und Fallgedächtnis
- Experimenteller Custom-Firmware-Modus mit Codegenerierung und isolierter Prüfung
- Live-Monitor für Messwerte, Belege, Entscheidungen, generierten Code und Flashprotokolle
- Offline-Hilfe und signierte Aktualisierungen über WLAN

## Sicherheitsgrundsätze
1. Alle Diagnosezugriffe zunächst lesend.
2. Schreibbefehle nur an eindeutig identifiziertes, explizit freigegebenes Ziel.
3. Systemlaufwerke und Recovery-Bootmedium besonders schützen.
4. Kein blindes Durchprobieren inkompatibler Firmware.
5. Kein automatischer Flash aufgrund einer KI-Vermutung. Erforderlich: technische Kompatibilitätsprüfung und separate ausdrückliche Freigabe.
6. Keine Garantie, dass ein nicht ansprechbarer oder signaturgesperrter Controller wiederherstellbar ist.
7. Datenrettung hat Vorrang vor potenziell destruktiven Controller-Neuinitialisierungen.

Siehe [Master-Spezifikation](docs/MASTER_SPECIFICATION_DE.md), [Architektur](docs/ARCHITEKTUR_DE.md), [Sicherheit](docs/SICHERHEIT_DE.md), [Roadmap](docs/ROADMAP_DE.md).

## Rechtliches
Proprietäre Firmware, Treiber und Tools werden nur bei erlaubter Weitergabe eingebunden. Noch keine Firmwareimages, Modelle oder bootfähigen Builds enthalten.

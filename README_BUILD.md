# NVMe Recovery Studio AI 0.1.0

Erste ausführbare Desktop-Grundlage: grafische Oberfläche, lesende Inventarisierung und lokaler JSON-Export. Windows 10/11: nur USB-Datenträger sichtbar. Linux: NVMe und USB.

**Nicht implementiert:** KI, SMART, Firmware-Recovery, Datenrettung, Flashen und bootfähiges Linux-Image.

Start: `python app/main.py`.

Windows-Installer: GitHub Actions `Windows Installer` baut mit PyInstaller und Inno Setup einen Setup-Installer und stellt ihn als Workflow-Artefakt bereit. Keine Installerdatei als fertig bezeichnen, bevor der Build erfolgreich gelaufen ist.

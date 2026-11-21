# NVMe Recovery Studio AI – Entwicklungsstand 0.5.0 (GitHub)

## Tatsächlich vorhandene Funktionen
- Grafische Desktopoberfläche und lesende Laufwerksinventarisierung.
- Windows 10/11: ausschließlich USB-Laufwerke in der Geräteauswahl. Die USB-Anbindung allein beweist noch nicht, dass eine NVMe-SSD dahintersteckt.
- Linux: direkte NVMe-Namespaces und USB-Laufwerke in der Inventarisierung.
- Windows: zusätzlicher lesender USB-Verbindungsbericht mit erneuter Geräteidentitätsprüfung. USB-Anbindung ist kein Nachweis für NVMe-Passthrough.
- Rechneranalyse mit Betriebssystem-, CPU- und optionalen PCI/USB-Informationen.
- Linux-NVMe-Diagnose mit `nvme-cli`: SMART, Controller-Identify und Fehlerlog als JSON, sofern Berechtigungen und Gerätezugriff vorhanden sind.
- Lokale SQLite-Fallakten: speichern, auflisten und öffnen.
- JSON-Export inklusive vorhandener Diagnosedaten, wahlweise mit Ausblendung von Seriennummern und Gerätepfaden.
- Regelbasierte Offline-Auswertung von NVMe-SMART-Werten mit Belegen und Unsicherheiten.
- Vergleich gespeicherter und aktueller SMART-Messwerte nur bei übereinstimmender Geräteidentität.

## Nicht vorhanden
Noch kein generatives lokales KI-Modell, keine Datenrettung, kein Firmware-Flash, kein Controller-Recovery, kein bootfähiges Linux-Image. Kein fertiger oder verifizierter Installer.

## Start
`python app/main.py`

Unter Linux ist für die NVMe-Diagnose zusätzlich `nvme-cli` erforderlich. Fehlende Berechtigungen und nicht unterstützte Geräte werden als Fehler angezeigt.

## Windows-Installer
Der GitHub-Actions-Workflow `Windows Installer` ist für PyInstaller und Inno Setup vorgesehen. Ein Setup-EXE darf erst nach erfolgreichem Build als verfügbar bezeichnet werden. Die Zielgröße für das spätere einzelne Installationspaket beträgt maximal 250 MB; eine Überschreitung erfordert vorherige Freigabe.

## Weitere Ergänzungen
- Geräteidentität wird vor Linux-NVMe-Abfragen erneut anhand von Seriennummer und Größe verglichen.
- Rechneranalyse läuft ohne Blockieren der Oberfläche.
- Bedienelemente sind in zwei Reihen angeordnet.
- Der Installer-Workflow begrenzt die Ausgabe auf genau eine EXE mit maximal 250 MB.

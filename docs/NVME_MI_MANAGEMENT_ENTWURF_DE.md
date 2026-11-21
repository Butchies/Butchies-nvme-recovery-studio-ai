# NVMe Management Interface (NVMe-MI) – Produktentwurf

## Ziel
Die Anwendung soll NVMe-MI als eigene Verwaltungs- und Diagnosefähigkeit berücksichtigen, ohne daraus pauschal eine verfügbare Schnittstelle für jede SSD abzuleiten.

## Fähigkeiten und Erkennung
- Anzeigen, ob NVMe-MI-Funktionen am konkreten Gerät und über den aktuellen Anschluss tatsächlich zugänglich sind.
- Unterschied zwischen gewöhnlichen NVMe-Adminbefehlen und Management-Interface erklären.
- Verfügbare Managementinformationen, Gerätekennungen und Statusmeldungen als lesende Datenquellen kennzeichnen.
- Transport, Bridge, Betriebssystem und Hardwarevoraussetzungen sichtbar machen.
- Nicht erreichbare oder herstellerspezifische Funktionen als unbekannt beziehungsweise nicht verfügbar ausweisen.

## Oberfläche
- NVMe-MI-Steckbrief innerhalb der Geräte-Detailansicht.
- Übersicht „verfügbar“, „nicht verfügbar“, „nicht unterstützt“, „nicht geprüft“.
- Quellenangabe je gemeldeter Fähigkeit.
- Verknüpfung mit Controllerwissen, Diagnosebericht und Firmwarebibliothek.

## KI-Verhalten
- NVMe-MI-Ergebnisse als zusätzliche Evidenz behandeln, nicht als Ersatz für SSD-Identität oder Datenrettung.
- Fehlende NVMe-MI-Antworten nicht automatisch als Controllerdefekt interpretieren.
- Keine unbekannten Managementaktionen automatisch ausführen.
- Änderungen am Gerätezustand nur in getrennten, eindeutig gekennzeichneten Bereichen anbieten.

## Editionsgrenzen
- Windows 10/11: Recovery-Ziel weiterhin ausschließlich externe USB-NVMe-SSD.
- Linux-Bootedition: direkte PCIe-Verbindungen können zusätzliche Informationen ermöglichen, garantieren aber keinen NVMe-MI-Zugang.

## Noch gewünschte Funktionen
NVMe-MI-Fähigkeitsansicht, Verwaltungsinformationen und evidenzbasierte KI-Erklärungen.

## Quelle
NVM Express Specifications: https://nvmexpress.org/specifications/

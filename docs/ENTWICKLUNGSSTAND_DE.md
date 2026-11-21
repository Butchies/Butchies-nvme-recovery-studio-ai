# Entwicklungsstand (21.11.2026)

## Programmiert (native/, nicht gebaut, nicht auf Hardware getestet)
- Geräteerkennung Windows (USB) und Linux (PCIe/sysfs, Notmodus-Verdacht), Schutz von Boot-Stick/Systemlaufwerken.
- NVMe-Daten: Windows-Protokollabfrage, smartctl über USB-Bridges; Linux direkt per NVMe-Admin-Befehlen.
- KI-Agent: Diagnose, Vorlagenabgleich, Firmwarefehler, Verlauf, Reparaturwege, Rettungsplan, Ergebnisbewertung.
- Lernende Wissensbasis (`ki-wissen.json`), Vorlagen-Vorschläge in Quarantäne.
- Rettungsabbild (Windows/Linux) mit Lesekarte, Zeitlimits, Fortsetzen, SHA-256, Bericht.
- Firmware-Manager (Linux): Prüfungen, Freigabecode, Download/Commit/Reset, Kontrolle.
- Werkzeug-Manager: Erkennung, automatische smartmontools-Installation, Registrierung eigener Werkzeuge.
- Boot-Stick: live-build-Konfiguration mit Kiosk, WLAN, dauerhaftem Speicher; Stick-Ersteller (Windows) und make-stick.sh (Linux).

## Noch offen
- Bau und Hardwaretests beider Editionen; Windows-Installer für die Qt-Anwendung.
- Anbindung registrierter Notmodus-Werkzeuge (Aufruf mit Parametern je Werkzeug).
- Lokales Sprachmodell (optional) für freie Erklärungen.
- Mehrsprachigkeit, Handbücher mit echten Screenshots.

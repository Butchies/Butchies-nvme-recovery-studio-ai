# NVMe Recovery Studio AI – Oberflächenentwurf

Nur gewünschte Oberfläche und Funktionen, keine Implementierungs- oder Testanweisung.

## Grundgestaltung
- Hochwertige dunkle Desktopoberfläche mit klaren Kontrasten, ruhigen Farben und gut lesbarer Typografie.
- Großzügige Gerätekarten, eindeutige Statussymbole, nachvollziehbare Warnungen.
- Gleiche Designsprache für Windows und Linux; Linux direkt im Vollbild.
- Einfacher Modus für verständliche Erklärungen, Expertenmodus für technische Details.

## Hauptnavigation
1. **Dashboard:** Geräte, Status, letzte Fälle, Offline-/Online-Status.
2. **Geräte:** SSD-Liste, USB-Bridge, Controller, Kapazität, Firmware und Verbindung.
3. **Diagnose:** SMART, Fehlercodes, Logs, Firmware-Slots, bekannte Symptome.
4. **KI-Live-Monitor:** Beobachtungen, Hypothesen, Quellen, Entscheidungen, Ereignisverlauf.
5. **Firmware:** Firmwarebibliothek, Versionen, Herkunft, Kompatibilität und Download.
6. **Recovery:** verfügbare Lösungswege, Risiken, erforderliche Freigaben.
7. **KI-Labor:** experimentelle Entwicklung eigener Diagnose- und Controller-Firmware, Quelltext und Fortschritt.
8. **Fallakten:** frühere Geräte, Symptome, Maßnahmen, Erkenntnisse und Ergebnisse.
9. **Netzwerk:** WLAN und Online-/Offline-Ressourcen (insbesondere Linux).
10. **Einstellungen:** Sprache Deutsch/Englisch, Design, lokale Speicherorte, Datenschutz, Aktualisierungen.

## Dashboard
- Kopfbereich mit Programmname, Edition, Sprache, WLAN-/Netzwerkstatus und KI-Status.
- Hauptkarte für ausgewählte SSD.
- Gesundheits-/Diagnosestatus und wichtigste Hinweise.
- Schnellzugriffe auf Diagnose, KI-Monitor und Fallakten.
- Deutliche Anzeige, ob Daten real erfasst oder nur demonstriert werden.

## Geräte-Detailansicht
- Hersteller, Modell, Seriennummer, Kapazität, Verbindung und Controller.
- Erreichbare Funktionen der Verbindung.
- Hardware- und Firmwareinformationen nach Verfügbarkeit.
- Sichtbare Einschränkungen, wenn Angaben unbekannt sind.

## KI-Live-Monitor
- Linke Spalte: Ereignis-Zeitleiste.
- Mitte: aktuelle Diagnose, Messwerte und Begründungen.
- Rechte Spalte: Hypothesen, Quellen, Risiken und nächste Vorschläge.
- Expertenansicht zusätzlich mit Rohdaten, Codeänderungen und detaillierten Protokollen.

## Recovery-Ansicht
- Vorherige Diagnose und verfügbare Maßnahmen übersichtlich darstellen.
- Unterschied zwischen Datenrettung, Firmware-Reparatur und experimentellem Modus deutlich machen.
- Risiken und Benutzerfreigaben vor Schreibaktionen klar sichtbar.
- Nicht unterstützte Aktionen als solche kennzeichnen.

## KI-Labor
- Controllerprofil und bekannte technische Dokumentation.
- Projektübersicht der experimentellen Firmware.
- Quelltextansicht, Änderungsverlauf, Entwicklungsstatus und technische Grenzen.
- Keine Erfolgsgarantie und kein automatisches Flashen.

## Noch offen
Die gesamte grafische Anwendung und ihre Ansichten müssen durch die andere KI programmiert werden.

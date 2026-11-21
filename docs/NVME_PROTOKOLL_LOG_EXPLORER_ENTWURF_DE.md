# NVMe-Protokoll- und Log-Explorer – Produktentwurf

## Ziel
Ein eigener Explorer soll ausgelesene NVMe-Informationen verständlich darstellen und Rohdaten für Experten nachvollziehbar erhalten.

## Ansichten
- Controller- und Namespace-Identifikation mit Herkunft, Zeitstempel und Gerätezuordnung.
- SMART-/Health-Informationen, Fehlerprotokolle, Firmware-Slot-Informationen und unterstützte Merkmale, soweit verfügbar.
- Befehls- und Log-Kategorien mit Status: lesbar, nicht unterstützt, durch Bridge blockiert oder unbekannt.
- Rohwert, interpretierter Wert, Einheit und Erklärung nebeneinander.
- Protokollversion und gerätespezifische Abweichungen kenntlich machen.

## Vergleich und KI
- Zwei Aufnahmen desselben Geräts vergleichen, ohne geänderte Hardwareumgebung zu übersehen.
- Fehlerstatus und Controllerantworten erläutern, einschließlich der Grenzen der Interpretation.
- Beobachtungen von KI-Hypothesen getrennt anzeigen.
- Nicht standardisierte Herstellerdaten nur mit belegter Bedeutung interpretieren.

## Oberfläche
- Einfache Zusammenfassung und Expertenansicht.
- Suche nach Logseite, Kennung oder Begriff.
- Kopierbare Einzelwerte und exportierbarer Bericht.
- Verknüpfung mit Controlleridentifikation, Fallakte und Hardwareprofil.

## Schutz
- Standardmäßig nur lesende Informationen anzeigen.
- Keine unbekannten oder herstellerspezifischen Befehle automatisch auslösen.
- Fehlende Logdaten nicht als Beweis für einen SSD-Defekt behandeln.

## Noch gewünschte Funktionen
Log-Explorer, Rohwertanzeige, Interpretation, Versionshinweise und Vergleichsansicht.

# Datenrettung: Zielmedien und Speicherplanung – Produktentwurf

## Ziel
Vor dem Sichern von Daten einer gefährdeten NVMe-SSD soll die Anwendung verständlich anzeigen, wohin gerettete Daten geschrieben werden und ob das Ziel geeignet ist.

## Zielmedien-Auswahl
- Verfügbare, beschreibbare Zielmedien mit Name, Kapazität, freiem Platz und Anschluss anzeigen.
- Quell-SSD und Zielmedium grafisch eindeutig unterscheiden.
- Niemals die betroffene SSD selbst als Ziel für ihre Rettungsdaten anbieten.
- Linux-Bootmedium und geschützte Systemdatenträger besonders kennzeichnen.
- Bei unklarer Geräteidentität keine automatische Auswahl treffen.
- Unter Windows 10/11 interne Datenträger nur als ausdrücklich gewählte Speicherziele für gerettete Dateien berücksichtigen, niemals als SSD-Reparaturziele.

## Speicherbedarf
- Geschätzten Platzbedarf für Abbild, Dateien und zusätzliche Protokolle separat anzeigen.
- Unterschiede zwischen vollständigem Datenträgerabbild und ausgewählter Dateirettung erklären.
- Unbekannten Bedarf ehrlich kennzeichnen.
- Hinweis auf Dateisystemgrenzen, beispielsweise maximale Dateigrößen.
- Bei zu wenig freiem Speicher passende Alternativen vorschlagen.

## Benutzerführung
- Quelle → Rettungsart → Ziel → Übersicht → Start.
- Dateiname, Speicherort und vorhandene Dateien vorab anzeigen.
- Überschreiben vorhandener Rettungsdaten nicht stillschweigend zulassen.
- Während des Vorgangs Quelle, Ziel, gelesene Daten und verbleibenden Speicher anzeigen.
- Unterbrechungen und erneute Auswahl des Zielmediums nachvollziehbar machen.

## Ergebnis
- Gesicherte Dateien und Abbilder übersichtlich anzeigen.
- Unlesbare Bereiche und unvollständige Ergebnisse kenntlich machen.
- Ergebnis mit Fallakte und Sitzung verknüpfen.
- Kein „erfolgreich gerettet“ behaupten, wenn Daten nur teilweise lesbar sind.

## Noch gewünschte Funktionen
Zielmedienübersicht, Speicherplanung, Schutz vor falscher Zielauswahl und Ergebnisverwaltung.

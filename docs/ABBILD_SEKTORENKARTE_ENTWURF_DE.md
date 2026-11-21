# Datenträgerabbild und Sektorenkarte – Produktentwurf

## Ziel
Bei lesbaren oder teilweise lesbaren NVMe-SSDs soll die Anwendung den Zustand einer Datenkopie anschaulich zeigen. Der Benutzer soll wissen, welche Bereiche gesichert, ausgelassen, fehlerhaft oder noch unbekannt sind.

## Abbild-Übersicht
- Quell-SSD und getrenntes Zielmedium mit eindeutiger Identität anzeigen.
- Gewähltes Abbildformat, Speicherort und benötigten Platz anzeigen.
- Fortschritt nach tatsächlich gesicherten Bereichen darstellen.
- Gelesene Datenmenge von erfolgreich geschriebenen Zieldaten unterscheiden.
- Verbleibenden Speicherplatz und Unterbrechungen anzeigen.

## Grafische Sektorenkarte
- Blöcke für erfolgreich gelesene, noch ausstehende, nicht lesbare und bewusst ausgelassene Bereiche.
- Zoom vom gesamten Laufwerk auf kleinere Adressbereiche.
- Anzeige von Lesefehlern und wiederholten Abbrüchen, soweit protokolliert.
- Unbekannte Bereiche nicht als erfolgreich markieren.
- Karte beim Fortsetzen einer Sitzung erneut anzeigen.

## KI-Unterstützung
- Erklären, was eine unvollständige Abbilddatei für spätere Dateirettung bedeutet.
- Auffällige Regionen und Veränderungen zwischen Sitzungen zusammenfassen.
- Bei instabilen SSDs auf vorsichtige, datenerhaltende Vorgehensweisen hinweisen.
- Keine vollständige Wiederherstellung versprechen, solange Bereiche fehlen.

## Ergebnis und Fallakte
- Abbild, Sektorenkarte und Sitzungsprotokoll logisch verknüpfen.
- Zusammenfassung über lesbare und fehlende Bereiche exportieren.
- Nachfolgende Partitions- und Dateisystemanalyse bevorzugt auf der Kopie anbieten.
- Original-SSD nicht stillschweigend verändern.

## Noch gewünschte Funktionen
Abbildverwaltung, grafische Sektorenkarte, nachvollziehbare Fortschrittsanzeige und Fallaktenintegration.

# System-Eignung und lokale KI-Ressourcen – Produktentwurf

## Ziel
Die Anwendung soll vor rechenintensiven KI-Funktionen verständlich anzeigen, welche Fähigkeiten der aktuelle Rechner bietet und welche lokalen KI-Funktionen sinnvoll nutzbar sind.

## Hardwareprofil
- CPU-Modell und relevante Rechenfunktionen.
- Verfügbarer und belegter Arbeitsspeicher.
- Erkannte GPU und verfügbarer Grafikspeicher, soweit ermittelbar.
- Freier Speicherplatz für lokale Modelle, Firmwarebibliotheken, Fallakten und Diagnoseberichte.
- Aktuelle Systemauslastung und Temperaturhinweise, soweit verfügbar.
- Bei Linux-Bootbetrieb: verfügbare Ressourcen des gestarteten Systems und des persistenten Speicherbereichs.

## Eignungsanzeige
- Bereiche „Diagnose“, „Lokale KI“, „Firmwareanalyse“, „KI-Labor“ und „Datenrettung“ separat bewerten.
- Klare Statusangaben: verfügbar, eingeschränkt, zusätzliche Ressourcen nötig oder unbekannt.
- Die Bewertung soll sich auf tatsächlich bekannte Voraussetzungen beziehen, nicht auf erfundene Leistungsversprechen.
- Erklären, welche Einschränkungen durch zu wenig RAM, fehlenden Speicherplatz oder ungeeignete GPU entstehen können.

## KI-Modellverwaltung
- Vorhandene lokale Modelle mit Größe, Sprache, Version und Einsatzbereich anzeigen.
- Bei mehreren Modellen eine verständliche Auswahl ermöglichen.
- Kleinere lokale Modelle für schwächere Rechner als Option anbieten.
- Größere Modelle nur bei geeigneter Hardware als sinnvoll kennzeichnen.
- Optional herunterladbare Modelle mit Speicherbedarf und Herkunft anzeigen.
- Offline nutzbare Funktionen deutlich markieren.

## Betriebsansicht
- Ressourcenbedarf laufender KI-Aufgaben anzeigen.
- Benutzer kann rechenintensive Vorgänge erkennen und bei Bedarf unterbrechen.
- Der Live-Monitor zeigt, welches Modell für eine Analyse verwendet wurde.
- Hardwareprofil und Modellwahl in Fallakten dokumentieren, soweit für die Nachvollziehbarkeit relevant.

## Noch gewünschte Funktionen
System-Eignungsanzeige, Modellübersicht, Ressourcenstatus und KI-Empfehlungen zur vorhandenen Hardware.

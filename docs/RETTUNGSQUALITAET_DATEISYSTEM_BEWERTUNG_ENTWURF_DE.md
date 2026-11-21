# Rettungsqualität und Dateisystem-Bewertung

## Ziel
Die Anwendung soll die Qualität einer Datenrettung nicht allein anhand der Anzahl gefundener Dateien beurteilen. Sichtbar werden sollen belegte Lesbarkeit, Dateisystemkonsistenz und verbleibende Unsicherheiten.

## Bewertungsbereiche
- Erfasste und fehlende Bereiche des Quellabbilds.
- Erkannte Partitionen und Dateisystemstrukturen.
- Gefundene Dateinamen, Verzeichnisse und Metadaten.
- Tatsächlich extrahierte Dateiinhalte.
- Optional überprüfbare Dateiformate, Prüfsummen und beschädigte Inhaltsbereiche.
- Unbekannte oder nicht lesbare Bereiche klar von nachgewiesen intakten Bereichen trennen.

## Darstellung
- Übersicht mit getrennten Kennzahlen für Abbildabdeckung, Dateisystemstruktur und wiederhergestellte Dateien.
- Kategorien: vollständig lesbar, teilweise lesbar, nicht überprüfbar, beschädigt.
- Keine pauschale Aussage „100 % gerettet“, wenn nur Dateinamen oder Verzeichniseinträge bekannt sind.
- Einzelne Dateien mit Herkunftsbereich und Prüfergebnis verknüpfen, soweit technisch möglich.
- Fehlende Dateisysteminformationen nicht mit fehlenden Nutzdaten gleichsetzen.

## KI-Unterstützung
- Unterschiede zwischen logisch gelöschten Dateien, Dateisystemschäden und physisch nicht lesbaren Bereichen erläutern.
- Erklärungen zur Unsicherheit bei fragmentierten Dateien und Dateicarving anbieten.
- Prioritäten nach Nutzerwunsch und vorhandener Lesbarkeit unterstützen.
- Keine erfundenen Erfolgswahrscheinlichkeiten oder Integritätsgarantien.

## Export
- Verständlicher Rettungsqualitätsbericht mit Datenquelle und Zeitpunkt.
- Liste problematischer Dateien und unbekannter Bereiche.
- Persönliche Dateinamen vor einer externen Weitergabe ausblendbar.

## Noch gewünschte Funktionen
Qualitätsübersicht, Dateiintegritätsstatus, Abbildabdeckung und nachvollziehbarer Abschlussbericht.

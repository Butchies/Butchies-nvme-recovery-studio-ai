# SSD-Betriebszustand und Schreibschutz – Produktentwurf

## Ziel
Die Anwendung soll unterscheiden, ob eine SSD normal arbeitet, nur lesbar ist, sporadisch antwortet oder durch Controller- beziehungsweise Verbindungseinschränkungen nicht vollständig zugänglich ist.

## Zustandsanzeige
- Erreichbarkeit und letzter bestätigter Gerätezugriff.
- Gemeldete kritische Warnungen und verfügbare Gesundheitsdaten.
- Schreibschutz oder Read-only-Zustand, soweit tatsächlich feststellbar.
- Unterschiede zwischen Betriebssystem-Mountstatus, Dateisystemschreibschutz und SSD-internem Zustand.
- Zeitweise verschwundene Geräte und Controller-Resets als eigene Zustände.

## KI-Unterstützung
- Mögliche Ursachen eines Read-only-Zustands erklären, ohne sie als bestätigt darzustellen.
- Verbindungsfehler und Berechtigungsprobleme als alternative Erklärungen berücksichtigen.
- Warnen, dass ein Read-only-Zustand nicht automatisch bedeutet, dass alle Daten noch vollständig lesbar sind.
- Datenrettung und Abbild auf getrenntem Zielmedium priorisieren.

## Oberfläche
- Verständliche Statuskarte mit Zeitpunkt und Datenquelle.
- Vergleich des Betriebszustands über mehrere Sitzungen.
- Verknüpfung mit Namespace-, Partitions- und Dateisystemansicht.
- Keine „Schreibschutz entfernen“-Schnellaktion bei unklarer Ursache.

## Noch gewünschte Funktionen
Betriebszustandskarte, Schreibschutz-Einordnung, Zustandsverlauf und KI-Empfehlungen.

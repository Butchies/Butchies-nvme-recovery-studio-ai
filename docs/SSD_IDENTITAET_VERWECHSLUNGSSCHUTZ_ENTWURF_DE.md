# SSD-Identität und Verwechslungsschutz – Produktentwurf

## Ziel
Die Anwendung soll eine SSD über mehrere Sitzungen, Anschlüsse und Rechner hinweg wiedererkennen können, ohne unterschiedliche Laufwerke aufgrund gleicher Modellbezeichnungen zu verwechseln.

## Identitätsmerkmale
- Modellbezeichnung, Seriennummer, Firmwareversion und Kapazität.
- Controller- und Namespace-Kennungen, soweit verfügbar.
- USB-Bridge und Gehäuse als separate Geräteidentität.
- PCIe-Anschluss und Hardwarepfad nur als Kontext, nicht als alleinigen SSD-Beweis.
- Benutzerdefinierter Gerätename und dokumentierte physische Merkmale.

## Unsichere Fälle
- Fehlende oder fehlerhafte Seriennummern.
- Gleiche Modellbezeichnung bei mehreren SSDs.
- Wechsel des USB-Gehäuses oder der USB-Bridge.
- Controller-Recovery-Modus mit anderen Gerätekennungen.
- Geänderte Namespace-Struktur oder Kapazität.
- Teilweise identische Daten nach einem Firmwarewechsel.

## Benutzeroberfläche
- „Bekanntes Gerät“, „möglicherweise bekannt“ oder „nicht eindeutig zugeordnet“ anzeigen.
- Unterschiede zu früheren Geräteprofilen offenlegen.
- Manuelle Zuordnung nur mit sichtbarer Warnung und Dokumentation.
- Quelle, Rettungsziel und Bootmedium eindeutig unterscheidbar halten.
- Vor Datenrettung, Firmwaremaßnahme und anderen risikobehafteten Aktionen die Zuordnung erneut sichtbar machen.

## KI-Unterstützung
- Mehrere Identitätsmerkmale vergleichen statt einzelne Angaben zu überbewerten.
- Bei Widersprüchen die Fallakte nicht stillschweigend einem Gerät zuordnen.
- Alternative Erklärungen für Identitätswechsel anzeigen.
- Keine automatische Freigabe gefährlicher Aktionen bei unklarer Geräteidentität.

## Noch gewünschte Funktionen
Geräteidentitätskarte, Sitzungsvergleich, Zuordnungswarnungen und Verwechslungsschutz.

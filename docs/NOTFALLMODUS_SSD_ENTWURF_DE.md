# SSD-Notfallmodus – Produktentwurf

## Ziel
Bei Anzeichen eines instabilen oder ausfallenden NVMe-Laufwerks soll die Anwendung einen besonders vorsichtigen Bedienmodus anbieten, in dem Datenerhalt und verständliche Warnungen im Vordergrund stehen.

## Auslöser und Hinweise
- Wiederholte Verbindungsabbrüche oder Controller-Resets.
- Starke Häufung von Lesefehlern.
- Kritische Temperaturmeldungen.
- SSD nur zeitweise erreichbar.
- Benutzer kann den Notfallmodus auch selbst auswählen.
- Nicht jeder einzelne Fehler bedeutet automatisch einen bevorstehenden Totalausfall.

## Oberfläche
- Deutlich sichtbarer Status „SSD-Notfallmodus“.
- Zusammenfassung der beobachteten Risiken und fehlenden Informationen.
- Vorrangige Aktionen: Fallakte sichern, Rettungsziel auswählen, Datenrettung ansehen.
- Firmwareänderungen und andere potenziell destruktive Optionen deutlich getrennt darstellen.
- Einfache Hinweise, warum unnötige Zugriffe vermieden werden sollten.

## KI-Verhalten
- Schonende Optionen und Datenrettung priorisieren.
- Unsichere oder riskante Vorschläge nicht als Standardempfehlung anzeigen.
- Hinweise auf mögliche Hardware-Datenrettung geben, wenn Softwarezugriff nicht mehr sinnvoll erscheint.
- Beobachtungen und Empfehlungen nachvollziehbar dokumentieren.

## Benutzerentscheidungen
- Benutzer kann Notfallmodus aktivieren oder verlassen.
- Vor riskanten Aktionen gesonderte Warnung mit möglichen Auswirkungen.
- Keine eigenmächtigen Firmware-Schreibvorgänge.
- Unvollständige Datenrettungsergebnisse klar kennzeichnen.

## Noch gewünschte Funktionen
Notfalloberfläche, Warnlogik, datenerhaltende Empfehlungen und Fallaktenverknüpfung.

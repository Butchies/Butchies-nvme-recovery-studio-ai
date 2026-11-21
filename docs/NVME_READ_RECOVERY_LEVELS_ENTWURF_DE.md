# NVMe Read Recovery Levels – Produktentwurf

## Ziel
Die Anwendung soll vorhandene Informationen zu NVMe Read Recovery Levels verständlich darstellen. Diese optionale Fähigkeit betrifft den Zielkonflikt zwischen Lesedauer und Fehlerbehandlung, nicht eine garantierte Wiederherstellung.

## Fähigkeitsanzeige
- Unterstützung der Funktion nur bei tatsächlich bestätigter Controllerfähigkeit anzeigen.
- Vorhandene oder dokumentierte Recovery-Level und deren Bedeutung erklären.
- Zwischen Geräteangaben, tatsächlich beobachteten Lesezeiten und KI-Einschätzungen unterscheiden.
- Einschränkungen durch USB-Bridges, Treiber und Firmware sichtbar machen.
- Fehlende Unterstützung nicht als Defekt werten.

## Datenrettungsbezug
- Erklären, dass aggressivere Leseversuche bei instabilen SSDs zusätzliche Belastung und Zeit bedeuten können.
- Mögliche Zielkonflikte zwischen schneller Teilrettung und aufwendiger Fehlerbehandlung zeigen.
- Bereits gesicherte Bereiche in der Abbildkarte berücksichtigen.
- Keine eigenmächtige Änderung von Controller-Einstellungen oder automatische Wiederholungsserie.
- Bei drohendem Datenverlust die vorhandenen sicheren Rettungsoptionen priorisieren.

## KI-Erklärung
- Für vorgeschlagene Wege die Datenlage, Risiken und Grenzen nennen.
- Keine Erfolgsquote oder wiederherstellbare Datenmenge erfinden.
- Unterschied zwischen optionaler NVMe-Funktion und tatsächlich unterstütztem Gerät erläutern.

## Oberfläche
- Informationskarte im NVMe-Funktionsprofil.
- Verknüpfung mit Abbild, Sektorenkarte, Notfallmodus und Diagnosebericht.
- Expertenansicht mit vorhandenen Gerätewerten und Quellen.

## Noch gewünschte Funktionen
Read-Recovery-Funktionsprofil, Risikoerklärung und Integration in die Datenrettungsplanung.

## Quelle
NVM Express: Changes in NVMe Revision 1.4 – Read Recovery Levels (optional):
https://nvmexpress.org/changes-in-nvme-revision-1-4/

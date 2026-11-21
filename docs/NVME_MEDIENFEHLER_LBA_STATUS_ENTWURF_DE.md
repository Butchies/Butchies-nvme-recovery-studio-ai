# NVMe-Medienfehler und LBA-Status – Produktentwurf

## Ziel
Die Anwendung soll bekannte nicht lesbare oder potenziell nicht wiederherstellbare logische Blockbereiche verständlich darstellen, wenn die SSD entsprechende standardisierte Informationen bereitstellt.

## Informationen
- Vorhandene LBA-Statusinformationen getrennt von tatsächlich beobachteten Lesefehlern anzeigen.
- Betroffene logische Bereiche mit Startadresse, Länge und Quelle darstellen, soweit verfügbar.
- Controller-/Namespace-Zuordnung und logische Blockgröße berücksichtigen.
- Nicht gemeldete Bereiche nicht automatisch als fehlerfrei einstufen.
- Unterschiede zwischen Medienfehler, Verbindungsabbruch und Timeout erklären.

## Oberfläche
- LBA-Statuskarte als zusätzliche Ebene der Sektorenkarte.
- Markierung „vom Gerät gemeldet“, „beim Lesen beobachtet“ oder „noch unbekannt“.
- Filter nach Fehlerart und Sitzung.
- Verknüpfung mit Datenträgerabbild, Dateisystem und Rettungsergebnissen.

## KI-Unterstützung
- Mögliche Folgen für einzelne Dateien oder Partitionen nur mit vorhandener Zuordnung erläutern.
- Datenrettung auf Kopien priorisieren.
- Keine automatischen Schreibversuche auf problematische LBAs.
- Bei nicht unterstützter Funktion alternative verfügbare Diagnoseinformationen benennen.

## Grenzen
- LBA-Statusfunktionen sind nicht bei jeder NVMe-SSD verfügbar.
- USB-Bridges können den Zugriff verhindern.
- Fehlende Statusmeldungen sind kein Nachweis einer fehlerfreien SSD.

## Noch gewünschte Funktionen
LBA-Fehlerkarte, Zuordnung zu Abbildern und verständliche KI-Erklärungen.

## Referenz
NVM Express: Changes in NVMe Revision 1.4, Abschnitt Rebuild Assist:
https://nvmexpress.org/changes-in-nvme-revision-1-4/

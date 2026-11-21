# Persistentes NVMe-Ereignisprotokoll – Produktentwurf

## Ziel
Ein spezieller Ereignisbereich soll wichtige Vorgänge aus dem Persistent Event Log verständlich anzeigen, wenn das konkrete Gerät diesen optionalen NVMe-Mechanismus unterstützt.

## Ereignisarten
- Neustarts, Stromzyklen und Hardwarefehler, soweit gemeldet.
- Firmware-Commit-Ereignisse und Versionswechsel.
- Änderungen an Namespaces.
- Start und Abschluss von Formatierungs- oder Sanitize-Vorgängen.
- Temperaturereignisse und vorhandene Gesundheits-Snapshots.
- Unbekannte und herstellerspezifische Ereignisse als nicht interpretiert kennzeichnen.

## Oberfläche
- Chronologische Zeitleiste mit Typ, Zeitpunkt, Quelle und Detailansicht.
- Deutlich zwischen Gerätezeit, Rechnerzeit und nicht verlässlich zuordenbarer Zeit unterscheiden.
- Filter nach Ereigniskategorie und Relevanz.
- Vergleich mit Rechnerereignissen und früheren Fallakten.
- Rohdaten nur mit dokumentierter Bedeutung interpretieren.

## KI-Unterstützung
- Mögliche Zusammenhänge zwischen Ausfällen, Firmwarewechseln und Verbindungsproblemen erklären.
- Nicht behaupten, dass das Protokoll alle historischen Ereignisse vollständig enthält.
- Hinweise auf bereits gestartete datenvernichtende Aktionen sichtbar machen.
- Ereignisse als Belege in Diagnose und Entscheidungshilfe verknüpfen.

## Grenzen
- Fehlende Unterstützung oder durch USB-Bridge blockierte Logseiten transparent anzeigen.
- Das Lesen optionaler Logs bei instabiler SSD gegen zusätzlichen Zugriff abwägen.
- Keine schreibenden Aktionen aus der Ereignisansicht heraus automatisch auslösen.

## Noch gewünschte Funktionen
Persistente Ereignischronik, Filter, Geräte-/Host-Zeitvergleich und KI-Erklärung.

## Referenz
NVM Express, Changes in NVMe Revision 1.4: https://nvmexpress.org/changes-in-nvme-revision-1-4/

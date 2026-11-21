# NVMe-Telemetrie und Ereignischronik – Produktentwurf

## Ziel
Zusätzlich zu gewöhnlichen SMART-Werten soll die Anwendung erweiterte NVMe-Telemetrie und relevante Ereignisse verständlich aufbereiten, soweit Controller, Treiber und Anschluss dies zulassen.

## Datenansicht
- Unterstützte Host- und Controller-Telemetrie getrennt aufführen.
- Verfügbare Logbereiche, Größe, Erfassungszeitpunkt und Quelle anzeigen.
- Fehlerereignisse, kritische Warnungen und Controllerzustandswechsel chronologisch darstellen.
- Fehlende oder herstellerspezifisch verschlüsselte beziehungsweise undokumentierte Inhalte als nicht interpretierbar kennzeichnen.
- Zwischen vorhandenen Rohdaten und tatsächlich entschlüsselbaren Informationen unterscheiden.

## KI-Analyse
- Ereignisse mit Temperatur, Stromversorgung, USB-/PCIe-Abbrüchen und SSD-Zustand korrelieren.
- Alternative Ursachen und Datenlücken sichtbar machen.
- Keine Diagnose aus undokumentierten Bytes erfinden.
- Telemetrieänderungen mit früheren Sitzungen vergleichen.

## Oberfläche
- Zeitachse mit Filtern nach Warnung, Fehler, Verbindung und Quelle.
- Detailkarte mit Rohwert, verständlicher Beschreibung und Evidenz.
- Optionaler Export geeigneter Rohdaten mit Datenschutzwarnung.
- Verknüpfung mit Fallakte, Diagnosebericht und Rechneranalyse.

## Schutz und Grenzen
- Telemetrieerfassung nicht mit einem risikofreien Zugriff gleichsetzen, wenn eine SSD instabil ist.
- Im SSD-Notfallmodus zusätzliche Zugriffe nur nach verständlicher Abwägung anbieten.
- Keine Behauptung, dass jede USB-Bridge alle Logseiten durchreicht.

## Noch gewünschte Funktionen
Telemetrieübersicht, Ereignischronik, Quellennachweis und KI-Korrelation.

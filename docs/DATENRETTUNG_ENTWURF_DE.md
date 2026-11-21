# Datenrettung und SSD-Reparatur – gewünschter Funktionsumfang

Dies ist ausschließlich der Produktentwurf für die andere KI, kein Programmier- oder Testplan.

## Zwei unterschiedliche Ziele
**Daten retten:** Vorhandene Dateien, Partitionen und Dateisysteme möglichst erhalten oder auf ein anderes Medium kopieren.

**SSD reparieren:** Den Datenträger wieder funktionsfähig machen. Firmware-Reparatur oder Neuinitialisierung kann vorhandene Daten zerstören.

Das Programm muss beide Ziele verständlich unterscheiden und darf sie nicht als dasselbe behandeln.

## Gewünschte Funktionen
- Benutzer wählt zuerst, ob vorhandene Daten wichtig sind.
- Anzeige, ob eine SSD lesbar, teilweise lesbar oder nicht zugänglich ist.
- Soweit technisch möglich: Vorschau erkannter Partitionen, Dateisysteme und Datenbereiche.
- Möglichkeit, lesbare Bereiche auf ein separates Zielmedium zu sichern.
- Fortschrittsanzeige für Kopier- und Wiederherstellungsvorgänge.
- Umgang mit Lesefehlern und unterbrochenen Vorgängen, soweit möglich.
- Wiederaufnahme einer unterbrochenen Datensicherung anhand gespeicherter Sitzungsinformationen.
- Verständliche Unterscheidung zwischen Dateiwiederherstellung, Rohdatenkopie und Controller-/Firmware-Reparatur.
- Keine Sicherung auf die beschädigte Quell-SSD als Standardziel.
- Deutliche Warnung vor Aktionen, die Datenzugriff verschlechtern oder Daten löschen können.

## Wenn eine SSD nicht mehr erkannt wird
- Anzeige, welche Informationen überhaupt noch ermittelbar sind.
- Hinweise auf USB-Bridge-Einschränkungen und möglichen direkten PCIe-Zugriff.
- Erklären, wenn eine reine Softwarelösung nicht ausreicht.
- Keine garantierte Datenrettung bei zerstörter Hardware oder unzugänglicher Verschlüsselung versprechen.

## Gewünschte Oberfläche
- Eigener Bereich „Datenrettung“.
- Geräte- und Zielmediumauswahl mit klarer Kennzeichnung.
- Ansicht für erkannte Bereiche, Sicherungsfortschritt und Ergebnis.
- Verknüpfung mit KI-Diagnose und lokalen Fallakten.

## Noch offen
Der gesamte Bereich Datenrettung einschließlich seiner Oberfläche und Funktionen muss von der anderen KI entwickelt werden.

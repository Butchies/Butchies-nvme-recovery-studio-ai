# Hardwareprofile und Rechnervergleich – Produktentwurf

## Ziel
NVMe Recovery Studio AI soll mehrere Rechner- und Anschlusskonfigurationen als getrennte Hardwareprofile erfassen können. Die KI soll erkennen, ob ein SSD-Problem nur auf einem bestimmten Rechner, USB-Gehäuse oder Anschluss auftritt.

## Hardwareprofile
- Rechnername als frei wählbare Bezeichnung; keine Pflicht zur Speicherung personenbezogener Daten.
- Betriebssystem, CPU, RAM, Mainboard, Chipsatz, relevante USB-/PCIe-Controller und Treiberversionen.
- Erfasste Anschlussart mit Port, USB-Bridge, Gehäuse und erreichbaren NVMe-Funktionen.
- Zeitpunkt der Aufnahme und Hinweis, welche Werte unbekannt sind.
- Hardwareprofile lokal speichern, umbenennen, exportieren und löschen.

## Vergleichsansicht
- Zwei oder mehrere Hardwareprofile nebeneinander betrachten.
- Unterschiede bei Controller, Verbindungsgeschwindigkeit, Treiber und Bridge hervorheben.
- Vergleichen, welche SSD-Daten in welchem Aufbau auslesbar waren.
- Sichtbar machen, ob dieselbe SSD in einem anderen Gehäuse oder Rechner anders reagiert.
- Zeitliche Veränderungen innerhalb desselben Rechners anzeigen.

## KI-Auswertung
- Erkennen, ob beobachtete Probleme eher an einer bestimmten Anschlusskette hängen.
- Hypothesen aus Profilunterschieden ableiten und verständlich erklären.
- Kein eindeutiger Hardwaredefekt allein aus einem Profilunterschied behaupten.
- Vorschläge für passende weitere Verbindungsoptionen und Datenrettungswege anzeigen.

## Fallakten
- Hardwareprofile mit SSD-Fallakten verknüpfen.
- Verbindungswechsel als eigenen Eintrag im Verlauf dokumentieren.
- Exportierbare Vergleichszusammenfassung mit optional anonymisierten Rechnerdaten.

## Noch gewünschte Funktionen
Profilverwaltung, Profilvergleich, KI-Interpretation und Verknüpfung mit SSD-Fallakten.

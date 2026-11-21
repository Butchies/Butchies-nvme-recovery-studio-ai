# USB-Bridge-Firmware und SSD-Firmware unterscheiden

## Ziel
Die Windows-Edition soll klar trennen, ob eine Störung im USB-NVMe-Gehäuse, dessen Bridge-Controller oder in der NVMe-SSD selbst liegt. Insbesondere darf eine Bridge-Firmware nicht mit der SSD-Controller-Firmware verwechselt werden.

## Zwei getrennte Geräteebenen
- USB-Bridge: Hersteller, USB-Produktkennung, Firmwarestand und unterstützte Weitergabe von NVMe-Befehlen, soweit auslesbar.
- NVMe-SSD: Modell, Seriennummer, Controller, Namespace und SSD-Firmwarestand.
- Fehlende SSD-Informationen können durch Bridge-Einschränkungen entstehen; nicht automatisch als SSD-Defekt werten.
- Eine Bridge kann mehrere SSD-Modelle unterstützen, ohne dass ihre Firmware mit der SSD-Firmware kompatibel wäre.

## Fehleranalyse
- Verbindungsausfälle, ungewöhnliche Geschwindigkeiten, fehlende SMART-Werte und blockierte Adminbefehle getrennt einordnen.
- Vergleich derselben SSD in einem anderen Gehäuse oder direkt unter Linux ermöglichen.
- Bridge-spezifische Fehlerhistorie mit Host-Hardwareprofil und SSD-Fallakte verknüpfen.
- Ursachenhypothesen mit konkreten Beobachtungen begründen.

## Firmware- und Updatehinweise
- Bridge-Firmware und SSD-Firmware in zwei getrennten Informationskarten führen.
- Updates für Bridge und SSD niemals austauschbar darstellen.
- Herstellerfreigaben, Boardrevisionen, USB-IDs und Versionsgrenzen berücksichtigen.
- Kein generisches Bridge-Update und kein automatisches Flashen als Diagnoseaktion.
- Bei unbekannter Bridge-Firmware ausdrücklich „nicht sicher identifiziert“ anzeigen.

## Oberfläche
- Grafische Kette: Windows-PC → USB-Port/Kabel → Bridge → NVMe-SSD.
- Pro Element eigene Identität, Fehlerhinweise und dokumentierte Fähigkeiten.
- Deutlicher Hinweis, wenn für bestimmte NVMe-Funktionen direkter PCIe-Zugriff über die Linux-Edition erforderlich ist.

## Noch gewünschte Funktionen
Getrennte Firmwarekarten, Bridge-Fehlerprofil, Verbindungsvergleich und eindeutige Updatewarnungen.

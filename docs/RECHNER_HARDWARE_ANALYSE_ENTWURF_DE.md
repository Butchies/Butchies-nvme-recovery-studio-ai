# Rechner- und Hardwareanalyse – Produktentwurf

## Ziel
NVMe Recovery Studio AI soll neben der SSD auch den verwendeten Rechner und seine relevante Hardware untersuchen können. Die KI soll dadurch erkennen, ob ein Problem möglicherweise durch den PC, dessen Anschluss, Treiber, Stromversorgung, Temperatur oder die USB-/PCIe-Verbindung entsteht, statt vorschnell die SSD als defekt einzustufen.

## Rechnerübersicht
- Betriebssystem und Version: Windows 10/11 oder Linux-Recovery-System.
- CPU-Modell, Architektur, Arbeitsspeicher und verfügbare Systemressourcen.
- Mainboard-/Systemmodell, BIOS-/UEFI-Version und relevante Plattforminformationen, soweit auslesbar.
- Chipsatz, PCIe-Controller, NVMe-Controller und zugehörige Treiber.
- USB-Controller, USB-Port-Typ, Verbindungsgeschwindigkeit und erkannte USB-NVMe-Bridges.
- Verfügbare Hardwarefunktionen für Diagnose, KI-Modell und Recovery.
- Anzeige unbekannter oder nicht verfügbarer Angaben statt Schätzwerte.

## Verbindungs- und Hardwarezustand
- Prüfen, ob ein NVMe-Gerät über die erwartete USB- oder PCIe-Verbindung erreichbar ist.
- Hinweise auf zu geringe USB-Verbindungsgeschwindigkeit, Verbindungsabbrüche oder Bridge-Einschränkungen.
- PCIe-Linkgeschwindigkeit und Linkbreite, soweit verfügbar.
- Systemseitige Fehlerhinweise und relevante Treiber-/Controllerereignisse.
- Temperaturinformationen des Rechners und der SSD, soweit verfügbar.
- Stromversorgungs- oder Energiesparprobleme als mögliche Ursache kenntlich machen, wenn entsprechende Hinweise vorliegen.
- Unterschiede zwischen SSD-Defekt und Problemen der Host-Hardware nachvollziehbar erklären.

## KI-gestützte Einordnung
- Rechnerhardware, Verbindung und SSD-Befunde gemeinsam betrachten.
- Mögliche Ursachen wie inkompatibles USB-Gehäuse, fehlerhafter Port, Treiberproblem, thermisches Problem oder tatsächlicher SSD-Fehler unterscheiden.
- Technische Grenzen der aktuellen Hardware anzeigen.
- Empfehlungen für geeignetere Anschlüsse, Adapter oder den Wechsel zur Linux-PCIe-Edition geben.
- Festhalten, welche Schlussfolgerungen auf Messwerten beruhen und welche nur Vermutungen sind.

## Eigener Programmbereich „Rechneranalyse“
- Rechnerübersicht mit Hardwarekomponenten.
- Anzeige der erkannten USB-/PCIe-Topologie.
- Verbindungsstatus und auffällige Ereignisse.
- Bewertung der Eignung des Rechners für die geplanten Diagnose- und Recovery-Funktionen.
- Übergabe der Erkenntnisse an die KI-Diagnose und den Live-Monitor.
- Lokale Speicherung relevanter Ergebnisse in der Fallakte.

## Grenzen und Schutz
- Rechneranalyse zunächst lesend und ohne eigenmächtige Änderungen an BIOS, Treibern oder Systemeinstellungen.
- Windows-Edition bleibt bei SSD-Recovery auf externe USB-NVMe-Geräte beschränkt; interne Laufwerke dürfen nur als allgemeine Rechnerhardware erfasst werden, nicht als Reparaturziele.
- Keine Belastungstests oder Schreibzugriffe auf gefährdete SSDs ohne gesonderte Benutzerentscheidung.
- Keine unbelegten Aussagen über elektrische Defekte oder Spannungen, wenn entsprechende Messdaten fehlen.

## Noch offen
Rechnerübersicht, Hardwareerkennung, USB-/PCIe-Topologie, Verbindungsbewertung, KI-Verknüpfung und grafische Rechneranalyse.

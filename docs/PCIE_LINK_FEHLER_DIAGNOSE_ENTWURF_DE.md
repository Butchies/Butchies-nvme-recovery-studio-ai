# PCIe-Link- und Signalfehlerdiagnose – Produktentwurf

## Ziel
In der Linux-Bootedition soll die Anwendung unterscheiden helfen, ob eine NVMe-SSD selbst defekt ist oder ob PCIe-Link, Steckplatz, Adapter oder Plattform die Kommunikation beeinträchtigen.

## Geräte- und Linkansicht
- Erkannte PCIe-Topologie mit Root-Port, Zwischenkomponenten und NVMe-Endgerät.
- Ausgehandelte Link-Geschwindigkeit und Lane-Breite, soweit verfügbar.
- Gemeldete maximale Fähigkeiten getrennt vom aktuellen Betriebszustand.
- Link-Neuaushandlungen, Geräteverluste und relevante PCIe-Fehlermeldungen mit Zeitstempel.
- Adapter, Riser und mechanische Verbindungen als mögliche Einflussfaktoren berücksichtigen.

## KI-Einordnung
- Zusammenhang zwischen Link-Problemen, Temperatur, Energieverwaltung und SSD-Zugriffsfehlern darstellen.
- AER-/Systemmeldungen nur entsprechend ihrer tatsächlichen Aussagekraft interpretieren.
- Link-Fehler nicht automatisch als NAND- oder Firmwaredefekt ausgeben.
- Fehlende Plattformtelemetrie ausdrücklich kennzeichnen.
- Vergleich mit früheren Rechner- und Anschlussprofilen anbieten.

## Benutzeroberfläche
- Grafischer Verbindungspfad mit markierten Auffälligkeiten.
- Detailkarte mit ausgehandeltem Link, Ereignissen und Quellen.
- Hinweise zu alternativen Anschlüssen ohne Erfolgsgarantie.
- Fallakten- und Hardwarebericht-Verknüpfung.

## Editionsgrenze
- Direkte PCIe-NVMe-Diagnose nur in der Linux-Bootedition.
- Unter Windows 10/11 bleiben ausschließlich externe USB-NVMe-SSDs Diagnose- und Recovery-Ziele; interne PCIe-Laufwerke dürfen nicht als solche angeboten werden.

## Noch gewünschte Funktionen
PCIe-Link-Dashboard, Fehlerchronik, Ursachenvergleich und verständliche KI-Erläuterung.

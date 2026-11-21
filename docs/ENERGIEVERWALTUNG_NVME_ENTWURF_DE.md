# NVMe-Energiezustände und Verbindungsstabilität – Produktentwurf

## Ziel
Die Anwendung soll Hinweise auf Probleme beim Wechsel zwischen Energiespar- und Betriebszuständen erkennen und verständlich von eigentlichen SSD-Hardwarefehlern abgrenzen.

## Informationen
- Gemeldete NVMe-Energiezustände und deren Eigenschaften, soweit lesbar.
- Hinweise auf Host-Energiesparmechanismen, USB-Suspend und PCIe-Link-Energiesparen.
- Ereignisse rund um Aufwachen, Wiederverbinden, Timeouts und Controller-Resets.
- Tatsächlich gemessene Werte von theoretischen Gerätefähigkeiten unterscheiden.
- Treiber-, Betriebssystem- und Bridge-Einflüsse dokumentieren.

## Darstellung
- Zeitachse: Verbindungsstatus, Last, Temperatur und relevante Energieereignisse.
- Gegenüberstellung verschiedener USB-Ports, Gehäuse oder Rechnerprofile.
- Klarer Hinweis, wenn eine Einstellung nicht ausgelesen werden kann.
- Verständliche Erklärung von APST, ASPM und USB-Energiesparen ohne unbelegte Ursachenzuschreibung.

## KI-Unterstützung
- Energiemanagement als mögliche Ursache sporadischer Ausfälle berücksichtigen.
- Alternative Ursachen wie Kabel, Bridge, Stromversorgung oder Controllerdefekt nebeneinander darstellen.
- Änderungen an Systemeinstellungen nicht ungefragt durchführen.
- Bei instabiler SSD unnötige wiederholte Zugriffe vermeiden.

## Noch gewünschte Funktionen
Energieereignis-Ansicht, Verbindungsvergleich und KI-Erklärungen zu Energiesparzuständen.

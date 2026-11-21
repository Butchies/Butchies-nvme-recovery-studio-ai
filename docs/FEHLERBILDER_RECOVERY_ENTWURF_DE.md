# Fehlerbilder und Recovery – Produktentwurf

Nur gewünschter Funktionsumfang aus Anwendersicht. Keine Vorgaben zu Tests oder konkreten Implementierungsschritten.

## Typische Fehlerbilder, die das Programm berücksichtigen soll
- SSD wird weder im Betriebssystem noch im BIOS/UEFI erkannt.
- SSD wird nur sporadisch erkannt oder verschwindet unter Last.
- Falsche Kapazität oder nur noch Controller-/Bootmodus sichtbar.
- SSD meldet kritische SMART-/Health-Werte.
- Fehlerhafte oder unvollständige Firmwareaktualisierung.
- NVMe-Timeouts, Controller-Resets oder Kommunikationsfehler.
- Nur lesbarer Modus, Schreibfehler oder Namespace-Probleme.
- Überhitzung oder auffällige Leistungs- und Verbindungsprobleme.
- Daten sind nicht erreichbar, obwohl die Hardware noch erkannt wird.
- USB-Gehäuse oder Bridge verhindert notwendige Diagnosefunktionen.

## Gewünschte Darstellung pro Fehlerbild
- Verständliche Symptombeschreibung.
- Beobachtete Gerätedaten und relevante Fehlercodes.
- Mögliche Ursachen mit kenntlich gemachter Unsicherheit.
- Welche Datenrettungs- und Reparaturmöglichkeiten grundsätzlich denkbar sind.
- Welche Voraussetzungen fehlen und welche Hardwaregrenzen bestehen.
- Klare Unterscheidung zwischen Wiederherstellung des Datenträgers und Rettung der vorhandenen Daten.

## Gewünschte Lösungswege
- Rein lesende Diagnose und Fehleranalyse.
- Hinweise auf geeignete Verbindung bzw. direkten PCIe-Zugriff, wenn USB begrenzt.
- Datenerhaltende Recovery-Optionen bevorzugt darstellen.
- Passende Original-Firmware identifizieren, falls verfügbar und kompatibel.
- Herstellerbezogene Wiederherstellungswege erläutern, soweit rechtmäßig verfügbar.
- Experimentelles KI-Labor bei fehlenden bekannten Lösungen als gesonderten Bereich anbieten.
- Vor potenziell zerstörerischen Aktionen Folgen und Freigabebedarf anzeigen.

## Nicht als garantiert darstellen
- Datenrettung bei physisch zerstörten NAND-Chips.
- Wiederherstellung unbekannter proprietärer Controller.
- Umgehung von Signatur- oder Verschlüsselungsbeschränkungen.
- Erfolgreiche Reparatur durch beliebige Firmware einer ähnlichen SSD.

## Noch offen
Fehlerdatenbank, Verknüpfung mit Gerätedaten, Recovery-Ansichten und konkrete gerätespezifische Unterstützung.

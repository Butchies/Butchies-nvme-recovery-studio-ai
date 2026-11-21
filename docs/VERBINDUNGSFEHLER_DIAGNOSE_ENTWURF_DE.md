# Verbindungsfehler und Ursachenvergleich – Produktentwurf

## Ziel
Bei intermittierenden oder fehlenden NVMe-Verbindungen soll die Anwendung verschiedene Ursachen verständlich unterscheiden, statt jeden Ausfall automatisch als SSD-Defekt zu behandeln.

## Fehlerbilder
- SSD erscheint nicht in der Geräteliste.
- SSD wird nur zeitweise erkannt.
- Verbindung fällt während eines Lesevorgangs aus.
- SSD wird mit unerwartet niedriger Geschwindigkeit angebunden.
- USB-Bridge meldet nur eingeschränkte Gerätedaten.
- Gerät meldet Timeouts, Resets oder I/O-Fehler.
- Gerät verschwindet nach Erwärmung oder Energiesparwechsel.

## Gegenüberstellung
- Hostcontroller, Port, USB-Kabel, Gehäuse und Bridge getrennt betrachten.
- Treiber- und Betriebssystemmeldungen in die Erklärung einbeziehen.
- Unter Linux direkte PCIe-Anbindung und Linkstatus berücksichtigen.
- Unterscheiden, ob das Gerät elektrisch erkannt, logisch identifiziert und tatsächlich lesbar ist.
- Kennzeichnen, wenn eine Zuordnung wegen fehlender Daten nicht möglich ist.

## Verlauf
- Zeitpunkte von Verbindungsaufbau, Ausfall und Wiedererkennung anzeigen.
- Ereignisse mit Temperatur, Verbindungsgeschwindigkeit und Diagnoseaktionen verknüpfen, soweit vorhanden.
- Unterschiede zwischen mehreren Sitzungen sichtbar machen.
- Ereignisverlauf als Teil der Fallakte exportieren.

## Empfehlungen
- Nur passende, nachvollziehbare nächste Schritte vorschlagen.
- Bei möglichem Kabel-, Port- oder Bridgeproblem entsprechende Alternativen erklären.
- Bei kritischen SSD-Symptomen zuerst Datensicherung beziehungsweise Datenrettung empfehlen.
- Vor riskanten Aktionen auf Datenverlustgefahren hinweisen.
- Grenzen softwareseitiger Diagnose offen darstellen.

## Noch gewünschte Funktionen
Verbindungsverlauf, Ursachenvergleich, Fehlerbildanzeige und KI-gestützte Empfehlungen.

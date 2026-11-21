# Externe Hardwaremesswerte und Elektronikdiagnose – Produktentwurf

## Ziel
Bei elektrischen Defekten sollen Benutzer Messwerte aus geeigneten externen Messgeräten ergänzen können. Die KI darf nur Werte bewerten, die tatsächlich vorliegen.

## Messwertarten
- Versorgungsspannung und Stromaufnahme, jeweils mit Einheit und Messpunkt.
- Auffällige Erwärmung und Temperaturmessungen.
- Durchgangs- und Widerstandsmessungen, soweit vom Benutzer bereitgestellt.
- Sichtbare Beschädigungen, Korrosion und Bauteilauffälligkeiten.
- Optional Fotos von Platine und markierten Messpunkten.

## Erfassung
- Manuelle Eingabe mit Messgerät, Messbereich, Einheit und Zeitpunkt.
- Herkunft „manuell gemessen“, „externes Gerät“ oder „geschätzt“ unterscheiden.
- Mehrere Messungen und Hardwarezustände nebeneinander anzeigen.
- Keine Spannungs- oder Stromwerte aus gewöhnlichen SSD-Softwaredaten erfinden.
- Optionale Geräteeinbindung nur bei tatsächlich unterstützten Messgeräten anzeigen.

## KI-Unterstützung
- Messergebnisse zusammen mit SSD-Symptomen, Verbindungsereignissen und Hardwareprofil betrachten.
- Auffälligkeiten und alternative Ursachen als Hypothesen kennzeichnen.
- Keine Bauteilreparatur oder Lötmaßnahme allein aufgrund eines unsicheren Messwerts als sicher darstellen.
- Bei Kurzschlussverdacht oder unbekannten Messpunkten auf professionelle Elektronikdiagnose hinweisen.
- Elektrische Diagnose von Firmware- und Dateisystemfehlern abgrenzen.

## Oberfläche
- Hardwareansicht mit Messpunktliste und Fotozuordnung.
- Zeitverlauf wiederholter Messungen.
- KI-Erklärung zu Messwert, Quelle und Aussagegrenze.
- Export in Fallakte und Rechner-/SSD-Gesamtbericht.

## Noch gewünschte Funktionen
Messwertverwaltung, Fotozuordnung, Elektronikbefunde und evidenzbasierte KI-Einordnung.

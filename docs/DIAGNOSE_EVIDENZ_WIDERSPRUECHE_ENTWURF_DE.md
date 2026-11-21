# Diagnose-Evidenz und widersprüchliche Befunde – Produktentwurf

## Ziel
Die KI soll widersprüchliche Messwerte und Beobachtungen ausdrücklich darstellen, statt vorschnell eine eindeutige Defektursache zu behaupten.

## Befundtypen
- Direkt ausgelesene SSD-Daten und NVMe-Logs.
- Betriebssystem- und PCIe-/USB-Ereignisse.
- Manuell eingetragene Messwerte und Gerätefotos.
- Ergebnisse früherer Sitzungen und anderer Rechner.
- Herstellerdokumentation und lokale Wissensdatenbank.
- KI-Hypothesen klar von Beobachtungen trennen.

## Widerspruchsansicht
- Gegenüberstellung „spricht dafür“, „spricht dagegen“, „noch ungeklärt“.
- Beispiel: SSD meldet unauffällige Gesundheitsdaten, aber Host protokolliert wiederholte Verbindungsabbrüche.
- Beispiel: Gerät erscheint im USB-Gehäuse mit anderer Identität als direkt am PCIe-Anschluss.
- Beispiel: Temperaturwerte unterscheiden sich zwischen Gerätelog und externer Messung.
- Zeitpunkte, Quellen und Messbedingungen berücksichtigen.

## KI-Verhalten
- Keine künstlichen Prozentwerte für Fehlerursachen ohne belastbare Datengrundlage.
- Alternative Hypothesen sichtbar halten.
- Änderungen der Einschätzung mit neuen Belegen dokumentieren.
- Fehlende Daten als fehlend kennzeichnen, nicht als Entwarnung.
- Für kritische Empfehlungen die maßgeblichen Beobachtungen benennen.

## Oberfläche
- Evidenztafel innerhalb jeder Fallakte.
- Filter nach SSD, Rechner, Verbindung und Zeitraum.
- Kurze verständliche Zusammenfassung plus technische Belegdetails.
- Exportierbare Belegübersicht ohne verborgene Gedankengänge zu behaupten.

## Noch gewünschte Funktionen
Widerspruchsmatrix, Quellenvergleich, zeitliche Nachvollziehbarkeit und transparente KI-Hypothesen.

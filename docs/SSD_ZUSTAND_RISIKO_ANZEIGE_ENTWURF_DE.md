# SSD-Zustand, Risikostufen und Handlungsübersicht – Produktentwurf

## Ziel
Vor Diagnose-, Datenrettungs- und Firmwareaktionen soll die Anwendung den bekannten Gerätezustand und mögliche Risiken in einer zentralen Übersicht verständlich darstellen.

## Zustandsbereiche
- Erreichbarkeit des Controllers und der Namespaces.
- Lesbarkeit und Stabilität der Verbindung.
- Gesundheitsdaten und Fehlerprotokolle, soweit verfügbar.
- Hinweise auf kritische Temperaturen, wiederholte Resets oder Lesefehler.
- Verfügbarkeit von Firmwareinformationen und Recovery-Zugriff.
- Kennzeichnung „unbekannt“, wenn keine zuverlässigen Daten vorliegen.

## Risikodarstellung
- Einfache Zustände wie „unauffällig“, „auffällig“, „kritisch“ und „nicht bewertbar“.
- Jede Bewertung mit sichtbaren Befunden begründen.
- Datenverlustrisiko von der Wahrscheinlichkeit einer erfolgreichen Reparatur unterscheiden.
- Keine garantierten Erfolgsquoten und keine künstliche Sicherheit vermitteln.

## Handlungskarten
- „Daten zuerst sichern“ bei vorhandener Lesbarkeit und möglicher Verschlechterung.
- „Verbindung überprüfen“ bei Hinweisen auf Host-, Kabel- oder Bridge-Probleme.
- „Diagnose vertiefen“ bei unvollständigen Informationen.
- „Firmwareinformationen ansehen“ bei passenden Controllerhinweisen.
- „Keine softwareseitige Lösung erkennbar“ bei entsprechenden Grenzen.

## Benutzerführung
- Aktuellen Zustand auf Dashboard und Gerätedetailseite anzeigen.
- Vor riskanten Schreibaktionen die mögliche Auswirkung auf Nutzerdaten deutlich darstellen.
- Änderungen der Risikobewertung im Fallverlauf festhalten.
- Expertenansicht mit den zugrunde liegenden Messwerten und Ereignissen.

## Noch gewünschte Funktionen
Zustandsübersicht, Risikobegründungen, Handlungskarten und historischer Verlauf.

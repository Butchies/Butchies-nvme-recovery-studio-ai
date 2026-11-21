# NVMe-Telemetrie-Snapshots und Konsistenz – Produktentwurf

## Ziel
NVMe Recovery Studio AI soll Host- und Controller-Telemetrie als nachvollziehbare Momentaufnahmen verwalten. Die Anwendung darf eine unvollständige oder zwischenzeitlich geänderte Datensammlung nicht als konsistenten Controllerzustand ausgeben.

## Funktionsumfang
- Host-initiierte und Controller-initiierte Telemetrie getrennt kennzeichnen.
- Unterstützte Telemetry Data Areas, Datenumfang und vorhandene Generation Numbers darstellen.
- Zeitpunkte, Gerät, Controller, Namespace-Bezug und Verbindung dokumentieren.
- Vorhandene Snapshots vergleichen und Abweichungen als zeitliche Veränderungen kennzeichnen.
- Unterschied zwischen dem Lesen vorhandener Telemetrie und dem Auslösen einer neuen Host-Aufnahme erläutern.

## Konsistenz und Vollständigkeit
- Zu Beginn und Ende eines Auslesevorgangs relevante Generationsangaben vergleichen.
- Wenn sich die Generation während des Abrufs ändert, den Snapshot als möglicherweise inkonsistent markieren.
- Fehlende Datenbereiche, Übertragungsabbrüche und Bridge-Beschränkungen sichtbar machen.
- Nicht dokumentierte Herstellerdaten nicht automatisch interpretieren.
- Ein nur teilweise vorliegendes Paket nicht als vollständiges Geräteabbild bezeichnen.

## Oberfläche
- Snapshot-Galerie mit Zeit, Quelle, Umfang und Konsistenzstatus.
- Vergleichsansicht mit unveränderten, geänderten und nicht vergleichbaren Feldern.
- Rohdaten- und Belegverknüpfung im Expertenbereich.
- Einbindung in Fallakten und KI-Live-Monitor.

## Datenschutz und KI
- Telemetriedaten lokal halten; Weitergabe an Hersteller ausschließlich nach ausdrücklicher Entscheidung.
- Sensible oder herstellerspezifische Inhalte vor Export kenntlich machen.
- KI darf nur belegte und interpretierbare Felder als Fakten ausgeben.
- Unbekannte Herstellerformate als Rohdaten mit Herkunft und Unsicherheit belassen.

## Noch gewünschte Funktionen
Snapshot-Verwaltung, Konsistenzanzeige, Generationenvergleich und kontrollierter Export.

## Quelle
NVM Express Base Specification Revision 2.3, Abschnitt Telemetry Data Collection:
https://nvmexpress.org/wp-content/uploads/NVM-Express-Base-Specification-Revision-2.3-2025.08.01-Ratified.pdf

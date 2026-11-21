# KI-Live-Monitor – detaillierter Produktentwurf

## Ziel
Benutzer sollen nachvollziehen können, was die KI über ein NVMe-Gerät weiß, welche Erklärungen sie für Fehler anbietet und welche Schritte sie vorschlägt.

## Ansichten
### Einfach
- Aktueller Vorgang in verständlicher Sprache.
- Erkannte Probleme und mögliche Ursachen.
- Fortschritt und nächster vorgeschlagener Schritt.
- Warnungen und offene Fragen.

### Experte
- Zeitgestempelte Ereignisse.
- Tatsächlich ausgelesene Gerätewerte und Fehlercodes.
- Datenquellen und technische Werkzeuginformationen.
- Hypothesen mit Belegen und Unsicherheiten.
- Vorgeschlagene Aktionen und nachvollziehbare Begründungen.
- Bei Firmwareentwicklung: Quelltext, Änderungen und Entwicklungsfortschritt.

## Ereigniskategorien
- Gerät erkannt oder entfernt.
- Hardwareinformation gelesen.
- Diagnosewert auffällig.
- Hypothese erstellt oder verworfen.
- Firmwareinformation gefunden oder nicht verfügbar.
- Benutzerentscheidung angefordert.
- Maßnahme gestartet, unterbrochen oder beendet.
- Fallakte aktualisiert.

## Bedienfunktionen
- Ereignisse nach Kategorie filtern.
- Details zu einzelnen Ereignissen öffnen.
- Ereignisverlauf pausiert betrachten, ohne den tatsächlichen Prozess anzuhalten.
- Verlauf lokal speichern und exportieren.
- Zwischen einfacher und Expertenansicht wechseln.

## Wichtige Darstellung
Es werden beobachtbare Daten, dokumentierte Schlussfolgerungen und erklärte Entscheidungen angezeigt. Verborgene interne Modellgedanken sind nicht auslesbar und werden nicht als solche behauptet.

## Noch offen
Die vollständige Live-Monitor-Oberfläche und die Verknüpfung mit realen Diagnosen, Fallakten und dem KI-Labor.

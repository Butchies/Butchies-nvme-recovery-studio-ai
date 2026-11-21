# Reparaturhistorie und KI-Lernen – Produktentwurf

## Ziel
Die KI soll nicht bei jedem neuen Fall wieder von vorne beginnen müssen. Frühere Diagnosen, beobachtete Fehler und bestätigte Ergebnisse sollen lokal auffindbar sein und spätere Lösungsvorschläge verbessern.

## Gewünschte Fallakte
- Fallnummer, Datum und frei wählbarer Titel.
- SSD-Modell, Controller, Firmwarestand und Verbindungstyp.
- Symptome und erfasste Diagnoseinformationen.
- KI-Hypothesen mit erkennbarer Unsicherheit.
- Vorgeschlagene und tatsächlich gewählte Maßnahmen.
- Beobachtete Ergebnisse und offene Fragen.
- Zugehörige Protokolle, Dateien und Notizen.
- Kennzeichnung, welche Erkenntnisse bestätigt und welche ungesichert sind.

## Lernfunktionen
- Ähnliche Fehlerfälle wiederfinden.
- Wiederkehrende Kombinationen aus Controller, Firmware und Fehlerbild erkennen.
- Frühere erfolgreiche und erfolglose Vorgehensweisen berücksichtigen.
- Neue Erkenntnisse für die lokale Wissensdatenbank vorschlagen.
- Widersprüchliche Erkenntnisse nebeneinander anzeigen und nachvollziehbar halten.
- Benutzer soll Fälle und gespeicherte Informationen verwalten und löschen können.

## Datenschutz
- Fallakten standardmäßig lokal.
- Export und Weitergabe nur auf Benutzerwunsch.
- Seriennummern und andere Gerätekennungen in Berichten bei Bedarf maskieren.
- Keine automatische Veröffentlichung von Daten.

## Gewünschte Oberfläche
- Fallübersicht mit Suche und Filtern.
- Detailseite mit Diagnoseverlauf, KI-Erklärungen und Ergebnissen.
- Vergleich ähnlicher Fälle.
- Verknüpfung mit KI-Live-Monitor und Controller-Wissensdatenbank.

## Noch offen
Die erste SQLite-Grundlage existiert bereits; die vollständige Fallverwaltung, Suche, Lernfunktionen und grafische Oberfläche fehlen.

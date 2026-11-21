# Rettungsziele und Speicherortverwaltung – Produktentwurf

## Ziel
Die Anwendung soll mehrere Speicherorte für Fallakten, Abbilder, Protokolle und gerettete Dateien verwalten und vor dem versehentlichen Schreiben auf die beschädigte Quell-SSD schützen.

## Speicherorttypen
- Externe USB-Datenträger und andere lokal erreichbare Zielmedien.
- Separate Speicherorte für Fallakten, große Abbilder und fertige Rettungsdateien.
- Verfügbare Kapazität, Dateisystem und Schreibbarkeit anzeigen.
- Netzwerkziele nur bei bewusster Einrichtung und bestehender Verbindung anbieten.
- Temporäre Speicherorte klar von dauerhaft nutzbaren Zielen unterscheiden.

## Sicherheitsregeln
- Quellgerät und alle zugehörigen Namespaces niemals als Rettungsziel vorauswählen.
- Bei unbekannter Gerätezuordnung keine Schreibfreigabe.
- Boot-USB-Stick der Linux-Edition vor unbeabsichtigter Nutzung als Ziel schützen.
- Vor dem Start Platzbedarf, freien Speicher und erwartete Reserven anzeigen.
- Bei fehlendem Platz oder unterbrochener Verbindung verständlich stoppen statt auf ein anderes Medium umzuschalten.
- Keine automatische Löschung vorhandener Abbilder zur Platzgewinnung.

## Oberfläche
- Speicherortübersicht mit eindeutiger Kennzeichnung von Quelle, Ziel und Bootmedium.
- Ordnerwahl, Speicherplatzprognose und Warnungen.
- Anzeige, welche Fallakten und Abbilder auf welchem Medium liegen.
- Fehlende Zielmedien nach Wiederanschluss anhand ihrer Identität erneut zuordnen.

## KI-Unterstützung
- Speicherplatzbedarf nur als Schätzung kennzeichnen.
- Geeignete getrennte Zielmedien empfehlen, ohne die Quell-SSD als Alternative vorzuschlagen.
- Bei unsicherem Zielmedium die Risiken erläutern.

## Noch gewünschte Funktionen
Zentraler Speicherortmanager, Quell-/Zielschutz, Kapazitätsplanung und Wiederanschluss-Erkennung.

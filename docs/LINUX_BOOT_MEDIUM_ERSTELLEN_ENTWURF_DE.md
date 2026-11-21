# Linux-Rettungsstick erstellen – Bedien- und Funktionsentwurf

## Ziel
Ein geführter Assistent soll einen bootfähigen USB-Stick für die Linux-Edition vorbereiten. Der Stick startet unmittelbar in die Vollbildoberfläche von NVMe Recovery Studio AI.

## Assistent
1. Geeignetes Linux-Rettungsabbild auswählen und Herkunft, Version sowie Prüfsumme anzeigen.
2. Angeschlossene USB-Datenträger mit Kapazität, Hersteller, Gerätename und Anschluss anzeigen.
3. Ausgewählten Zielstick eindeutig hervorheben; interne Datenträger niemals vorauswählen.
4. Vor dem Überschreiben die Datenlöschung des ausgewählten Sticks unmissverständlich ankündigen.
5. Fortschritt, Fehler, Abschluss und mögliche Startprobleme verständlich darstellen.
6. Nach Fertigstellung eine kompakte Anleitung für UEFI-Bootmenü und sichere Nutzung anbieten.

## Schutz vor Datenträgerverwechslung
- Zielmedium anhand mehrerer Merkmale identifizieren.
- Den USB-Rettungsstick von einer zu untersuchenden NVMe-SSD unterscheiden.
- Bei unklarer Zuordnung oder wechselnden Geräten keine Schreibfreigabe anbieten.
- Keine automatische Auswahl eines Datenträgers allein aufgrund seiner Größe.
- Die Erstellung des Bootsticks strikt von späteren SSD-Recovery-Aktionen trennen.

## Start und Kompatibilität
- UEFI- und Secure-Boot-Einschränkungen transparent erklären, ohne pauschale Kompatibilität zu versprechen.
- Falls das Startmedium auf einem Rechner nicht startet, verständliche Ursachen und alternative Startwege anzeigen.
- Hardwarevoraussetzungen für lokale KI und offline verfügbare Ressourcen vorab sichtbar machen.
- Kein regulärer Desktop als notwendiger Zwischenschritt: direkte Vollbild-Recovery-Oberfläche.

## Persistenz und Updates
- Klar unterscheiden, welche Fallakten und Einstellungen dauerhaft auf dem Stick gespeichert werden und welche nur temporär vorhanden sind.
- Keine stillschweigende Ablage sensibler Rettungsdaten auf dem Bootmedium.
- Updates dürfen vorhandene Fallakten nicht ungefragt überschreiben.
- Version und verfügbare Offline-Wissenspakete sichtbar machen.

## Noch gewünschte Funktionen
Bootstick-Assistent, eindeutige Medienauswahl, UEFI-Hilfe, Persistenzübersicht und sichere Aktualisierung.

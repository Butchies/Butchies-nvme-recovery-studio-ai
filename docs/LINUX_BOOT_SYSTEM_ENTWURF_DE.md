# Linux-Bootsystem – Produktentwurf

## Ziel
NVMe Recovery Studio AI soll als eigenständiges, bootfähiges USB-System verfügbar sein, um intern eingebaute NVMe-SSDs direkt über PCIe zu erreichen.

## Start und Oberfläche
- Start über UEFI vom vorbereiteten USB-Stick.
- Nach dem Start unmittelbar die NVMe-Recovery-Oberfläche im Vollbild anzeigen.
- Kein normaler Desktop als notwendiger Zwischenschritt.
- Startbildschirm mit Logo, Ladefortschritt und verständlichen Fehlermeldungen.
- Auswahl zwischen Deutsch und Englisch.
- Anzeige, ob das System online oder offline arbeitet.

## Hardware und Geräte
- Interne NVMe-Controller und Namespaces anzeigen.
- PCIe-Verbindung und verfügbare Hardwareinformationen darstellen.
- Mehrere SSDs unterscheiden und einzeln auswählen.
- Boot-USB-Stick und andere nicht ausgewählte Datenträger deutlich kennzeichnen.
- Gerätewechsel und Erkennungsprobleme verständlich anzeigen.

## Netzwerk
- WLAN-Netze suchen, verbinden und gespeicherte Verbindungen verwalten.
- Optional kabelgebundenes Netzwerk nutzen.
- Firmwareinformationen, Controllerwissen, KI-Ressourcen und Updates bei Bedarf nachladen.
- Basisfunktionen auch ohne Internet nutzbar halten.

## Offline-Ressourcen
- Lokale KI und lokale Wissensdatenbank.
- Vorhandene Firmwareinformationen und Hilfedaten.
- Fallakten, Diagnoseberichte und KI-Live-Monitor.
- Erkennbarer Status der installierten Offline-Pakete.

## Persönliche Daten
- Eigene Fallakten und Einstellungen über Neustarts hinweg erhalten.
- Optional verschlüsselter lokaler Speicherbereich für Fälle und persönliche Daten.
- Export auf ein separates Zielmedium anbieten.

## Gewünschtes Ergebnis
Ein selbstständiges USB-Recovery-System, das ohne installierte Windows-Anwendung gestartet werden kann und für die Diagnose intern verbauter NVMe-SSDs ausgelegt ist.

## Noch offen
Bootfähige Linux-Ausgabe, Vollbildoberfläche, WLAN-Verwaltung, persistente Daten und Offline-Ressourcen.

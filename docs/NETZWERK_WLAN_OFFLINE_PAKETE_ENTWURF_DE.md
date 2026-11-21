# WLAN, Netzwerk und Offline-Pakete – Produktentwurf

## Ziel
Die Linux-Bootedition soll WLAN nur für ausdrücklich gewünschte Aktualisierungen, Dokumentation und Downloads benötigen. Diagnose und lokal vorhandene KI-Funktionen bleiben ohne Internet zugänglich.

## Netzwerkoberfläche
- WLAN-Netze, Signalstärke und Verbindungsstatus anzeigen.
- Passwortgeschützte Verbindung über einen verständlichen Assistenten.
- Erkennbar machen, ob nur das lokale Netz oder auch das Internet erreichbar ist.
- Netzwerkausfälle und captive Portals ohne irreführende Erfolgsmeldung erklären.
- Nach Neustart gespeicherte Verbindungen nur entsprechend der gewählten Datenschutzeinstellung wiederverwenden.

## Paketverwaltung
- Getrennte Kategorien: KI-Modelle, Controllerwissen, Firmware-Metadaten, dokumentierte Firmwaredateien, Sprachpakete.
- Herkunft, Version, Größe, Lizenz und Prüfsumme anzeigen.
- Vor dem Download Speicherziel und freien Platz darstellen.
- Bereits vorhandene Pakete offline verwenden.
- Unterbrochene Downloads als unvollständig markieren; keine stillschweigende Nutzung beschädigter Pakete.
- Aktualisierungen nicht automatisch während einer laufenden Rettung erzwingen.

## Datenschutz
- Keine automatische Übertragung von Fallakten, Seriennummern, Dateinamen oder Diagnoseabbildern.
- Netzwerkzugriff und Datenfreigabe als zwei getrennte Entscheidungen.
- Deutlicher Hinweis, wenn eine externe Quelle kontaktiert wird.
- Offline-Modus als dauerhaft sichtbare Betriebsart.

## KI-Unterstützung
- Bei fehlender Verbindung mit lokalen Quellen weiterarbeiten.
- Fehlende aktuelle Quellen nicht als vorhandenes Wissen ausgeben.
- Online-Recherche nur nach Benutzerentscheidung; Ergebnisse mit Herkunft und Abrufdatum kennzeichnen.

## Noch gewünschte Funktionen
WLAN-Assistent, Offline-Status, Paketmanager und kontrollierte Online-Recherche.

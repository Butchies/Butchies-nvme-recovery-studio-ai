# Zoned Namespaces (ZNS) – Produktentwurf

## Ziel
Falls eine SSD das optionale Zoned-Namespaces-Befehlsmodell unterstützt, soll die Anwendung ihre Speicherorganisation korrekt darstellen und nicht wie bei einem gewöhnlichen blockbasierten Namespace interpretieren.

## Erkennung und Darstellung
- Gemeldetes I/O Command Set und Namespace-Typ anzeigen, soweit auslesbar.
- Normale NVM-Namespaces und Zoned Namespaces getrennt kennzeichnen.
- Zonen, Größen, Kapazitäten und Zustände nur mit belegten Gerätedaten darstellen.
- Unterschiede zwischen Zone, Namespace, Partition und Dateisystem erklären.
- Nicht unterstützte oder nicht erreichbare ZNS-Informationen als unbekannt anzeigen.

## Diagnose und Datenrettung
- ZNS-spezifische Zugriffsregeln bei der Erklärung von Befunden berücksichtigen.
- Abbild und Lesestatus weiterhin getrennt von schreibenden Zonenoperationen behandeln.
- Zonen-Reset, Zone-Finish und ähnliche verändernde Vorgänge nicht als harmlose Diagnose anbieten.
- Keine automatische Umwandlung oder Neuorganisation von Zonen.
- Grenzen der Dateisystem- und Dateirettung bei nichtstandardmäßigen Layouts erklären.

## KI-Unterstützung
- ZNS-Befunde nur bei bestätigtem Gerätetyp verwenden.
- Unbekannte Zoneninformationen nicht mit gewöhnlichen Partitionsdaten verwechseln.
- Empfehlungen auf tatsächlich unterstützte Fähigkeiten beschränken.
- Rohdaten und Quellen im Expertenmodus bereitstellen.

## Noch gewünschte Funktionen
ZNS-Erkennung, grafische Zonenansicht, Zugriffserklärungen und sichere Einbindung in die Recovery-Oberfläche.

## Quelle
NVM Express: Zoned Namespaces Command Set Specification:
https://nvmexpress.org/specification/zoned-namespaces-zns-command-set-specification/

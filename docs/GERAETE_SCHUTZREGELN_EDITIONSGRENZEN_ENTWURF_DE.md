# Geräteschutz und Editionsgrenzen – verbindlicher Bedienentwurf

## Ziel
Die Schutzregeln der Windows- und Linux-Edition müssen in allen Ansichten, Assistenten und KI-Vorschlägen gleich verständlich sein.

## Windows 10/11
- Nur **extern über USB angeschlossene NVMe-SSDs** dürfen als Diagnose-, Recovery- oder Reparaturziele erscheinen.
- Interne SSDs, Systemlaufwerke und andere interne Datenträger sind keine auswählbaren Recovery-Ziele.
- Rechnerhardware darf zur Ursachenanalyse berücksichtigt werden, ohne interne Laufwerke als Reparaturziele freizugeben.
- USB-Bridges und deren eingeschränkte NVMe-Befehlsweitergabe müssen sichtbar bleiben.
- Fehlt ein geeigneter USB-Zugriff, darf die Anwendung nicht stillschweigend auf interne Datenträger ausweichen.

## Linux-Bootedition
- Direkter PCIe-NVMe-Zugriff ist vorgesehen.
- Der laufende Rettungsstick und nicht ausgewählte Datenträger bleiben geschützt.
- System- und Nutzerdatenträger dürfen nicht allein aufgrund ihrer Erreichbarkeit automatisch verändert werden.
- Ein ausgewähltes Reparaturziel ist von Rettungsziel, Bootmedium und anderen angeschlossenen Geräten klar zu unterscheiden.
- Ein Laufwerkwechsel oder eine geänderte Identität erfordert eine erneute eindeutige Zuordnung.

## Gemeinsame Regeln
- Diagnose standardmäßig lesend.
- Datenrettung und Gerätereparatur als getrennte Aufgaben.
- Datenvernichtende Aktionen niemals als automatische Fehlerbehebung.
- KI-Empfehlungen sind keine Gerätefreigabe.
- Gefährliche Schritte nur mit sichtbaren Folgen und ausdrücklicher Zustimmung.
- Keine erfundene Sicherheit bei fehlenden Controller- oder Verbindungsinformationen.

## Oberfläche
- Dauerhaft sichtbare Editions- und Betriebsartenkennzeichnung.
- Auswahlansicht mit eindeutigem Symbol für Quelle, Ziel, Bootmedium und gesperrte Geräte.
- Verständliche Meldung, warum ein Gerät nicht auswählbar ist.
- Schutzstatus in Fallakte und Diagnosebericht dokumentieren.

## Noch gewünschte Funktionen
Einheitliche Schutzkennzeichnung, klare Gerätezuteilung und editionsübergreifende Bedienregeln.

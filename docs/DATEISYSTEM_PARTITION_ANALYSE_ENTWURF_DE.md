# Partitionen und Dateisysteme – Produktentwurf

## Ziel
NVMe Recovery Studio AI soll bei erreichbaren SSDs unterscheiden können, ob die Ursache eines Datenverlusts in der Hardware, Partitionstabelle, Dateisystemstruktur oder Verschlüsselung liegt.

## Übersicht
- Physisches Gerät, Namespace, Partitionen und Volumes getrennt anzeigen.
- Erkannte Dateisysteme und deren Zustand darstellen, darunter NTFS, exFAT, FAT32, ext4 und weitere, soweit erkennbar.
- Fehlende, beschädigte oder widersprüchliche Partitionseinträge als Befund anzeigen.
- Tatsächlich erfasste Daten von vermuteten Strukturen unterscheiden.
- „RAW“, nicht zugeordnet und nicht lesbar als unterschiedliche Zustände erläutern.

## KI-Analyse
- Symptome einer beschädigten Partitionstabelle von Controller- und Verbindungsfehlern abgrenzen.
- Mögliche Dateisystemschäden erklären, ohne voreilig Formatierung oder Reparaturschreibvorgänge zu empfehlen.
- Gefundene Dateisignaturen, Verzeichnisstrukturen und Metadaten nur als Hinweise bewerten.
- Priorisierte Datenrettung auf einer geeigneten Kopie oder einem Abbild anbieten.
- TRIM, Verschlüsselung und fehlende logische Blöcke als mögliche Grenzen darstellen.

## Benutzeroberfläche
- Grafische Datenträgerkarte mit Partitionen, belegtem und unbekanntem Bereich.
- Detailansicht je Partition mit bekannten Eigenschaften und Auffälligkeiten.
- Lesende Vorschau erkannter Dateien, soweit möglich.
- Klare Trennung zwischen Diagnose, Datenrettung und schreibenden Reparaturen.
- Ergebnis an Fallakte und Diagnosebericht übergeben.

## Grenzen
- Kein automatisches Initialisieren, Formatieren oder Reparieren des Originaldatenträgers.
- Keine Garantie auf wiederherstellbare Dateien allein wegen gefundener Dateinamen.
- Unter Windows 10/11 nur externe USB-NVMe-SSDs als Analysequelle der SSD-Recovery.

## Noch gewünschte Funktionen
Partitionsübersicht, Dateisystemdiagnose, KI-Erklärungen und sichere Übergabe an die Datenrettung.

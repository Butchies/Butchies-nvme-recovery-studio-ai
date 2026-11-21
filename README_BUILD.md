# NVMe Recovery Studio AI – Entwicklungsstand 0.25.0 (GitHub)

## Tatsächlich vorhandene Funktionen
- Grafische Desktopoberfläche und lesende Laufwerksinventarisierung.
- Windows 10/11: ausschließlich USB-Laufwerke in der Geräteauswahl. Die USB-Anbindung allein beweist noch nicht, dass eine NVMe-SSD dahintersteckt.
- Linux: alle erkannten physischen Laufwerke (intern und extern), damit auch direkte NVMe-SSDs und potenzielle M.2-SATA-SSDs erscheinen. M.2-SATA ist nicht immer von anderen SATA-Bauformen unterscheidbar; keine M.2-Bauformgarantie.
- Windows: zusätzlicher lesender USB-Verbindungsbericht mit erneuter Geräteidentitätsprüfung. USB-Anbindung ist kein Nachweis für NVMe-Passthrough.
- Konservative Schutzprüfung für gewählte Datenträger: erneute Identitätsprüfung, Linux-Mountpoints, aktive Boot-/System-Mounts, Swap-Hinweise und ausdrückliche Schreibsperre. Die Prüfung ist kein Freigabeverfahren für Schreibzugriffe.
- Lokaler Firmware-Metadatenkatalog (JSON, schema 1) mit SHA-256-Abgleich; Katalogeinträge sind nicht authentifiziert und erlauben keine Firmwareinstallation.
- Offline-Firmwaredatei-Inspektion: Dateigröße, SHA-256 und Header-Vorschau. Keine Ausführung, keine Signatur- oder Kompatibilitätsfreigabe und kein Flash.
- Linux: lesende Auswertung des Ziel-Mounts und der bekannten Blockgeräte-Abstammung; erkennt offensichtliche Quelle-Ziel-Überschneidungen, ohne eine sichere Trennung zu garantieren.
- Planung eines späteren Rohabbilds mit Kapazitätsprüfung und ausdrücklicher Sperre bis zur verifizierten Quell-/Zieltrennung; kein tatsächliches Imaging.
- Offline-Recovery-Leitfaden mit priorisierten, nicht ausführbaren Hinweisen auf Basis vorhandener Befunde.
- Rechneranalyse mit Betriebssystem-, CPU- und optionalen PCI/USB-Informationen.
- Linux-NVMe-Diagnose mit `nvme-cli`: SMART, Controller-Identify und Fehlerlog als JSON, sofern Berechtigungen und Gerätezugriff vorhanden sind.
- Linux-SATA/ATA-Diagnose mit `smartctl` (smartmontools): lesende SMART-Informationen, Gesundheitsstatus, Attribute und Fehlerlog, auch für potenzielle M.2-SATA-SSDs; keine M.2-Bauformbestätigung.
- Lokale SQLite-Fallakten: speichern, auflisten und öffnen.
- JSON-Export mit optionaler konservativer Anonymisierung: bekannte Kennungen und rohe Freitext-/Identifikationsdaten werden ausgeblendet. Vollständige Anonymität ist nicht garantiert.
- Regelbasierte Offline-Auswertung von NVMe-SMART-Werten mit Belegen und Unsicherheiten.
- Vergleich gespeicherter und aktueller SMART-Messwerte nur bei übereinstimmender Geräteidentität.

## Nicht vorhanden
Noch kein generatives lokales KI-Modell, keine Datenrettung, kein Firmware-Flash, kein Controller-Recovery, kein bootfähiges Linux-Image. Kein fertiger oder verifizierter Installer.

## Start
`python app/main.py`

Unter Linux ist für die NVMe-Diagnose zusätzlich `nvme-cli` und für SATA/ATA-SMART `smartmontools` erforderlich. Fehlende Berechtigungen und nicht unterstützte Geräte werden als Fehler angezeigt.

## Windows-Installer
Der GitHub-Actions-Workflow `Windows Installer` ist für PyInstaller und Inno Setup vorgesehen. Ein Setup-EXE darf erst nach erfolgreichem Build als verfügbar bezeichnet werden. Die Zielgröße für das spätere einzelne Installationspaket beträgt maximal 250 MB; eine Überschreitung erfordert vorherige Freigabe.

## Weitere Ergänzungen
- Geräteidentität wird vor Linux-NVMe-Abfragen erneut anhand von Seriennummer und Größe verglichen.
- Rechneranalyse läuft ohne Blockieren der Oberfläche.
- Bedienelemente sind in zwei Reihen angeordnet.
- Der Installer-Workflow begrenzt die Ausgabe auf genau eine EXE mit maximal 250 MB.

**Hinweis zur Schutzprüfung:** Die Erkennung von System- und Boot-Datenträgern ist bewusst konservativ und nicht vollständig (z. B. bei komplexen LVM-/RAID-/verschlüsselten Setups). Sie hebt niemals die Schreibsperre auf.

## Firmwarekatalog (nur Recherche)
Beispielstruktur: `{"schema":1,"entries":[{"sha256":"<64 Hexzeichen>","pci_vendor_id":"1234","pci_device_id":"abcd","model":"Beispiel","firmware_revision":"1.0","source":"Herstellerquelle dokumentieren"}]}`.
Ein Hash-Treffer im selbst bereitgestellten Katalog bestätigt weder Echtheit noch Controller-Kompatibilität. Kein automatischer Download und kein Flash.

## Wartung 0.10.0
- SQLite-Verbindungen nach Fallaktenoperationen ausdrücklich schließen; Fehler beim Öffnen und Speichern in der Oberfläche anzeigen.
- Datenschutzwarnung und restriktivere Ausblendung im anonymisierten JSON-Export.

## Wartung 0.11.0
- Fallaktenvergleich nutzt den aktuellen Diagnosebericht statt einer gespeicherten Hülle.
- Speichern ordnet die Geräteidentität dem tatsächlich erfassten Bericht zu, nicht einer eventuell später geänderten Auswahl.
- Vergleich lehnt fehlende SMART-Messwerte ab und weist auf rückläufige Zähler sowie mögliche Resets hin.

## Wartung 0.12.0
- Installer-GitHub-Workflow nur noch manuell auslösbar; Code-Commits erzeugen nicht automatisch ein Setup.
- Recovery-Leitfaden ist ausschließlich informativ. Keine Gerätebefehle oder automatischen Reparaturen.

## Wartung 0.13.0
- Neue Aktion »Abbild planen«: nur Größe und freien Speicher erfassen, keine Geräteöffnung, kein Schreiben, keine Abbilddatei.
- Die sichere Trennung von Quell- und Zieldatenträgern ist noch nicht nachgewiesen; der Plan erteilt niemals eine Ausführungsfreigabe.

## Wartung 0.14.0
- Zielverzeichnis wird unter Linux mit findmnt und lsblk rein lesend zugeordnet.
- Liegt das Ziel nachweisbar auf der gewählten Quelle, erscheint eine kritische Warnung.
- LVM, RAID, verschlüsselte, virtuelle und Netzwerk-Dateisysteme bleiben potenziell unvollständig auflösbar. Imaging weiterhin nicht freigegeben.

## Wartung 0.15.0
- Linux-Zielzuordnung ermittelt Elternbeziehungen aus dem lsblk-Gerätebaum statt aus möglicherweise fehlenden PKNAME-Feldern.
- Mehrfach-Elternbeziehungen werden berücksichtigt; Gerätepfade werden kanonisiert.
- Das Ergebnis ist weiterhin ausschließlich ein Warnhinweis und keine Imaging-Freigabe.

## Wartung 0.16.0
- Firmwaredateien werden über einen lesenden Dateideskriptor geöffnet, als reguläre Datei geprüft und auf 256 MiB begrenzt.
- Firmwarekataloge werden begrenzt auf 2 MiB eingelesen; Symlinks und Änderungen während des Lesens werden soweit durch das Betriebssystem unterstützt abgewehrt.
- Kein Firmware-Flash und keine Echtheits- oder Kompatibilitätsfreigabe.

## Wartung 0.17.0
- Windows-USB-Diagnose akzeptiert nur den kanonischen PHYSICALDRIVE-Pfad mit numerischer Laufwerksnummer.
- Abbildplanung läuft im Hintergrund, sodass Mount- und Kapazitätsabfragen die Oberfläche nicht blockieren.
- Ergebnisse werden verworfen, wenn zwischenzeitlich ein anderes Laufwerk ausgewählt wurde.

## Wartung 0.18.0
- Hintergrundaufgaben übergeben GUI-Ergebnisse jetzt an eine zentrale Warteschlange; Tk-Widgets werden im Hauptthread aktualisiert.
- Abbildplanung wird auch bei erneuter Inventarisierung verworfen, selbst wenn der Gerätepfad gleich geblieben ist.
- Es werden weiterhin keine Datenträger beschrieben, keine Firmware geflasht und keine Installer automatisch gebaut.

## Vorgabe 0.19.0: Linux intern und extern, Windows nur extern
- Linux inventarisiert alle physischen Datenträger, auch interne SATA-Geräte, weil sich M.2-SATA nicht sicher anhand von lsblk erkennen lässt.
- Windows bleibt bei externen USB-Datenträgern; interne Windows-Laufwerke werden nicht aufgelistet.
- Das Auflisten erteilt keine Schreibfreigabe; System- und Bootlaufwerke bleiben geschützt.

## Wartung 0.20.0
- Linux-Schutzprüfung unterstützt jetzt alle durch lsblk eindeutig als physischer Datenträger identifizierten Geräte, einschließlich SATA-M.2-Kandidaten; Schreibzugriffe bleiben immer gesperrt.
- NVMe-Namespaces werden anhand eines eindeutigen Gerätenamens statt einer unscharfen Teilzeichenfolge erkannt.

## Wartung 0.21.0
- Linux: SSD-Diagnose unterscheidet direkte NVMe- und SATA/ATA-Laufwerke und nutzt dafür jeweils nvme-cli bzw. smartctl.
- ATA-Diagnose prüft Gerätepfad, Transport, Seriennummer und Kapazität vor dem rein lesenden Abruf.
- USB-SATA-Bridges werden nicht als direktes ATA-Laufwerk ausgegeben. Keine SMART-Selbsttests, keine Reparatur und kein Schreiben.

## Wartung 0.22.0
- ATA-SMART-Befund bewertet einen vorhandenen SMART-Gesamtstatus und auffällige Sektor-Rohzähler (Attribute 5, 197, 198) mit Belegen und Unsicherheiten.
- Die Oberfläche zeigt den passenden ATA- oder NVMe-Befund, ohne SATA-Geräte fälschlich als NVMe zu bezeichnen.
- Herstellerabhängige SMART-Werte werden nicht pauschal als gesund oder defekt eingestuft. Kein Schreibzugriff.

## Wartung 0.23.0
- Recovery-Leitfaden berücksichtigt SATA/ATA-SMART-Warnungen, unvollständige smartctl-Ergebnisse und erkannte Quell-/Zielkonflikte.
- Der Leitfaden verarbeitet unvollständige oder unerwartete Berichtsstrukturen defensiver.
- Eine neue Abbildplanung bewahrt den bisherigen Diagnosebefund desselben Geräts, damit SMART-Auswertung und Leitfaden erhalten bleiben.
- Keine Datenträgeränderungen oder automatischen Reparaturen.

## Wartung 0.24.0
- Diagnose, Schutzprüfung, USB-Verbindungsbericht und Abbildplanung ergänzen nun eine gemeinsame Fallakte, sofern die Geräteidentität übereinstimmt.
- Gerätewechsel und erneute Inventarisierung machen ausstehende Hintergrundresultate ungültig; veraltete Befunde werden nicht mehr einem später ausgewählten Laufwerk zugeordnet.
- Weiterhin ausschließlich lesende Diagnose und Planung, keine Datenträger-Schreiboperationen.

## Wartung 0.25.0
- Gespeicherte ATA/SATA-Fallakten können jetzt anhand gemeinsamer numerischer SMART-Rohattribute verglichen werden.
- Ein Vergleich setzt übereinstimmende Seriennummer, Modell und Kapazität voraus; SATA- und NVMe-Befunde werden nicht miteinander vermischt.
- Gespeicherte verschachtelte Fallakten werden für den Vergleich entpackt; sinkende Rohzähler werden als mögliche Rücksetzung markiert, nicht als Heilung interpretiert.
- Keine Tests oder Datenträger-Schreiboperationen ausgeführt.

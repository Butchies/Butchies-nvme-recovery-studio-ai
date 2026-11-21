# Änderungsprotokoll

## 2026-11-21 – Controller-Vorlagen aller Hersteller
- `data/catalog/`: 121 Hersteller und 835 NVMe-Controller/-Produkte aus pci.ids (Stand 21.11.2026) inkl. Produkt-Zuordnungen, 223 USB-Speicher-/Bridge-Einträge aus usb.ids, Fachwissen zu 14 Controllerfamilien (Phison, Silicon Motion, Realtek, Maxio, InnoGrit, Samsung, WD/SanDisk, Kioxia, SK hynix/Solidigm, Micron/Crucial, Intel, Marvell, weitere), USB-NVMe-Bridges und belegte Firmwarefehler (Samsung 980 PRO/990 PRO, WD/SanDisk HMB 24H2).
- `tools/generate_catalog.py`: Vorlagen jederzeit aus den aktuellen Datenbanken neu erzeugen.
- `native/controller_catalog.h`: KI erkennt Controller-Hersteller (NVMe-Identify-VID), Controller/Produkt (exakte PCI-ID oder Namensabgleich), Bridge, SATA-Gehäuse, bekannte Firmwarefehler und leitet bewertete Reparaturwege ab.
- Neue Diagnose „SSD meldet sich nicht“ (Bridge ohne Medium) mit Reparaturwegen.
- Unbekannte Controller werden als Vorlagen-Vorschläge in Quarantäne gesammelt.
- Nicht gebaut und nicht auf Hardware getestet.

## 2026-11-21 – Lernende KI ab Programmstart
- KI-Agent startet mit dem Programm, überwacht USB-Anschlüsse (alle 4 s) und diagnostiziert neu angeschlossene SSDs automatisch.
- `native/knowledge_base.h`: lokale Wissensbasis (`ki-wissen.json` im AppData-Ordner) mit allen Diagnosen, Rettungen und Benutzeraktionen; Erfahrung je Gerät (Seriennummer nur als Hash), USB-Bridge und Controller-Modell.
- Diagnose nutzt Erfahrung: SMART-Verlauf (steigende Medienfehler, sinkende Reserve, wachsendes Fehlerprotokoll), bekannte Bridge-Grenzen, Vergleich mit früheren Fällen desselben Controllers.
- Rettungsplan passt sich an frühere Versuche an (Blockgröße, Zeitlimits, Wartezeit beim Neuverbinden, Fehlerbehandlung) und begründet jede Anpassung.
- KI-Live-Monitor in der Oberfläche: Erkennung, Belege, Warnungen, Entscheidungen, Ergebnisse und Gelerntes.
- Nicht gebaut und nicht auf Hardware getestet.

## 2026-11-21 – Automatische Rettung (Windows, Qt)
- Neue Schaltfläche „Automatisch retten“: frische Diagnose → Rettungsplan des Agenten → Rettungsabbild → Prüfung → Bewertung → Bericht, ohne weitere Benutzerentscheidungen.
- `native/rescue_imager.h`: Rettungsabbild in mehreren Durchgängen (Schnellkopie mit Sprüngen über Fehlerzonen, übersprungene Bereiche, Eingrenzen, Einzelsektoren, Wiederholungen). Zeitlimit je Lesebefehl, Pause bei wiederholten Zeitüberschreitungen, Neuverbindung bei kurzzeitiger Trennung.
- Lesekarte `abbild.map` (ddrescue-kompatibel) wird laufend gespeichert; Fortsetzen nach Abbruch oder Trennung mit demselben Zielordner. `herkunft.json` verhindert das Zusammenführen mit dem Abbild einer anderen SSD.
- Ziel wird geprüft: anderer physischer Datenträger als die Quelle, beschreibbar, ausreichend Platz, kein FAT32 bei mehr als 4 GiB.
- Nach Abschluss: Größenprüfung, SHA-256 (`abbild.img.sha256`), `bericht.txt`/`bericht.json`, `protokoll.log` mit allen Maßnahmen.
- `recovery_agent.h`: `planRescue` wählt Blockgröße, Sprungweiten, Zeitlimits und Wiederholungen nach SMART-/Fehlerzustand; `evaluateRescue` bewertet das Ergebnis und nennt die nächsten Schritte.
- Die SSD wird ausschließlich gelesen. Keine Firmware-, Reset-, Format- oder Sanitize-Befehle. Für den Rohzugriff fordert das Programm Administratorrechte an.
- Nicht gebaut und nicht auf Hardware getestet.

## 2026-11-21 – Übergabe auf Produktentwurf reduziert
- `AGENTS.md` enthält jetzt die verbindlichen Produktwünsche statt eines vorgeschriebenen KI-Programmierablaufs.
- `docs/AI_ENTWICKLUNGSPLAN_DE.md` beschreibt Vision, Funktionsumfang und noch offene Programmierarbeiten.
- Testpläne, Abnahmevorgaben und methodische Anweisungen für andere KIs aus diesen beiden Übergabedokumenten entfernt.
- Die konkrete technische Umsetzung und alle Prüfentscheidungen liegen bei der weiterentwickelnden KI.

## Vorheriger Stand
- Projektkonzept, Architektur und Sicherheitsanforderungen dokumentiert.
- Erste Linux-Inventarisierung, Planungslogik und SQLite-Fallakten als Codebasis vorhanden.
- Windows-/Linux-Komplettanwendungen und Installationspakete noch nicht fertig.

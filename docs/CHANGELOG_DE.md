# Änderungsprotokoll

## 2026-11-22 – Reparaturkette und Übersetzung der Controller-Meldungen
- `data/catalog/nvme_codes.json`: 45 NVMe-Statuscodes (allgemein, befehlsspezifisch, Flash/Datenintegrität, Verbindung), CSTS-Bits und Sonderwerte (z. B. 0xffffffff = SSD vom PCIe-Bus verschwunden), SMART-Warnbits, Selbsttest-Ergebnisse, 12 Linux-Kernelmeldungen (controller is down, Device not ready, I/O timeout, capacity change to 0, AER …) – jeweils mit verständlicher Übersetzung und Bedeutung.
- `native/error_translator.h`: „Was der Controller meldet“ – Fehlerprotokoll (bis 16 Einträge inkl. LBA), SMART-Warnungen und Kernelmeldungen zum Gerät, übersetzt, mit Schweregrad; Merkmale für die KI.
- `native/repair_chain.h` (Linux, Daten werden gelöscht): PCIe-Funktionsreset → PCIe entfernen/neu einlesen → Controller-Reset → anderen Firmware-Slot aktivieren → Format NVM mit Löschen (Verwaltungstabelle neu) → Sanitize Block Erase (mit Fortschritt) → Firmware neu aufspielen; nach jedem Schritt Prüfung (Identify, Kapazität, SMART, Oberflächen-Stichprobe), Abbruch bei Erfolg; Abschluss-Oberflächentest (Stichprobe oder ganze SSD). Ergebnis trainiert das KI-Modell.
- Oberfläche: „Reparieren“ (Linux), Schritte mit Controller-Antwort, Bereich „Was der Controller meldet“ (Rohmeldung + Übersetzung).
- Nicht gebaut und nicht auf Hardware getestet.

## 2026-11-22 – Cache-Test
- `native/cache_tester.h`: Cache-Ausstattung (DRAM oder DRAM-los/HMB, angeforderter und zugeteilter Host-Speicher, Schreibcache vorhanden/aktiv), NVMe-Controller-Selbsttest kurz/lang mit Ergebnis und Fehlersegment (Linux direkt, Windows über smartctl), Lesetest (wiederholt vs. zufällig, Ausreißer, sequenziell), Schreibtest nur Linux/Expertenmodus (Cache-Einbruch, Flush-Zeit; überschreibt Daten).
- Bewertung „Cache wird nicht angesprochen / defekt / instabil“; Ergebnis fließt in Wissensbasis und KI-Modell (neue Ursache „Cache defekt“, drei neue Merkmale; vorhandene Modelldatei wird dadurch neu initialisiert).
- Oberfläche: „Cache testen“, langer Selbsttest, Schreibtest (Linux), Fortschritt in Prozent, Ergebnis im Diagnosebereich.
- Nicht gebaut und nicht auf Hardware getestet.

## 2026-11-22 – Kleine lernende KI und Expertenmodus
- `native/case_model.h`: lernendes Ursachenmodell (logistische Regression, 31 Merkmale, 6 Ursachen: gesund, Flash-Verschleiß, Firmwarefehler, Bridge-Problem, Notmodus, Controllerausfall), startet mit Regelwissen und lernt aus jeder Rettung und jeder bestätigten Ursache; begründet jede Vorhersage. Rettungs-Tuner übernimmt Einstellungen der ähnlichsten erfolgreichen Rettung. Kein Sprachmodell, keine Grafikkarte nötig.
- Oberfläche: Vorhersage im Diagnosebereich, „Tatsächliche Ursache bestätigen – KI lernt“.
- Expertenmodus Linux-Boot-Edition: Firmware ohne Bestätigungscode und ohne Sicherungspflicht, Rettung pausiert nicht bei Zeitüberschreitungen. Gesperrt bleiben nur der laufende Boot-Stick und Ziel = Quelle.
- Windows unverändert: nur externe USB-SSDs.
- Nicht gebaut und nicht auf Hardware getestet.

## 2026-11-21 – Linux-Boot-Edition, Firmware-Manager, Boot-Stick
- `native/platform_io.h`: Rettungs-Ein-/Ausgabe für Windows und Linux (O_DIRECT + Kernel-AIO mit Zeitlimit); Zielprüfung unter Linux inkl. LVM/LUKS/RAID-Auflösung und Sperre für RAM-Dateisysteme des Live-Systems.
- `native/linux_nvme.h`: PCIe-/NVMe-Erkennung über sysfs, Notmodus-Verdacht, Schutz von Boot-Stick und Systemlaufwerken, NVMe-Admin-Befehle (Identify, SMART, Fehlerprotokoll, Firmware-Slots).
- `native/firmware_manager.h`: Pflichtprüfungen, Risiken, Bestätigungscode, Firmware-Download/Commit/Reset mit Prozentanzeige und anschließender Kontrolle.
- `native/stick_writer.h` + Oberfläche: Linux-Boot-Stick aus Windows erstellen (Laufwerke sperren, schreiben, per SHA-256 prüfen), neueste Version aus GitHub-Releases laden.
- `linux-boot/`: live-build-Konfiguration (Debian 13, UEFI/BIOS), Vollbild-Kiosk mit cage, WLAN, smartmontools/nvme-cli/ddrescue, dauerhafter Speicher, `make-stick.sh`.
- Agent und Oberfläche unterscheiden die Editionen.
- README und Entwicklungsstand aktualisiert.
- Nicht gebaut und nicht auf Hardware getestet.

## 2026-11-21 – Werkzeug-Manager und Downloadliste
- `data/catalog/tools.json` und `docs/WERKZEUGE_DOWNLOADLISTE_DE.md`: smartmontools, nvme-cli, GNU ddrescue, Samsung Magician, WD/SanDisk Dashboard, Crucial Storage Executive, Kingston SSD Manager, SK hynix Drive Manager, Solidigm Storage Tool, KIOXIA SSD Utility, Seagate SeaTools – jeweils mit offizieller Quelle; Notmodus-MPTools (Phison, SMI, Realtek, Maxio) als vom Nutzer bereitzustellende Werkzeuge.
- `native/tool_manager.h`: Erkennung installierter Werkzeuge, Registrierung eigener Werkzeuge, smartctl-Abfrage.
- smartmontools wird vom Programm selbst von der offiziellen GitHub-Release-Seite geladen und still installiert (Fortschritt in Prozent, Ladebalken alle 5 % im KI-Live-Monitor); übrige Werkzeuge öffnen die offizielle Seite, die Installation wird automatisch erkannt.
- Diagnose nutzt smartctl automatisch, wenn Windows über USB keine NVMe-Daten liefert; die KI lernt je Bridge das funktionierende Protokoll (sntjmicron/sntasmedia/sntrealtek).
- KI empfiehlt je Controllerfamilie die passenden Herstellerwerkzeuge.
- Nicht gebaut und nicht auf Hardware getestet.

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

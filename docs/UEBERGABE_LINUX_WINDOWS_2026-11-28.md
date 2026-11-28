# Übergabeprotokoll für die Weiterentwicklung · 28.11.2026

Repository: `Butchies/Butchies-nvme-recovery-studio-ai`, Zweig
`linux-recovery-updates`, Pull Request #1. Dieses Protokoll ist eine
Bestandsaufnahme. Maßgeblich sind der jeweilige Commit und die GitHub-Actions-
Protokolle; ein erfolgreiches Release oder ein USB-Hardwaretest darf nicht
allein aus diesem Text abgeleitet werden.

## Ausgangspunkt und Beobachtung

- Nutzergerät: Samsung/Dell PM981a 1 TB, Etikett `MZVLB1T0HBLR-000D1`,
  Dell DP/N `0F7DVX`, ursprüngliche Etikett-Firmware `15304129`.
- Letzte Hardwarebeobachtung: PCIe `0000:01:00.0`, `144d:a808` sichtbar,
  kein gebundener NVMe-Treiber und kein funktionierender `/dev/nvmeX`-
  Controllerknoten. PCIe-Reset und Neuscan halfen dabei nicht. Aktuelle
  Hardwaredaten müssen aus einem neu erzeugten Bootreport bestätigt werden.
- Eine PCI-ID beweist weder NAND-/Board-/SED-Kompatibilität noch die
  Verfügbarkeit eines herstellerspezifischen Notmodus-Flashverfahrens.

## Eingebaute Änderungen

1. Dell G4GWG wird beim ISO-Bau heruntergeladen und per SHA-256 geprüft.
   `MUP_PM981a_15305129.exe` wird daraus gesucht und erneut per SHA-256
   geprüft. Der Nutzlastbereich `[0x8d8d0:0x1cd8d0]` ergibt das 1.310.720
   Byte große `Dell-PM981a-G4GWG-15305129.bin` mit SHA-256
   `08156487302ee23413e2a6ceffc0ba1a94d9e881db55b407c0d81d79fa660db9`.
   Der Build bricht bei Abweichungen ab. Das Abbild ist eine Standard-NVMe-
   Firmware-Nutzlast, kein MPTool/ROM-Loader.
2. Das ISO enthält die Nutzlast mit `Firmware/PM981a/index.json`. Die
   Linux-App übernimmt das Archiv beim Start nur mit passender Prüfsumme.
   Der Flash-Manager verlangt NVMe Identify, Firmware-Download-Fähigkeit,
   Modellprüfung für dieses Abbild, ungeschützte und ausgehängte SSD sowie
   den expliziten Seriennummerncode, auch im Expertenmodus.
3. Der Build aktiviert jetzt `butchies-early-log.service` und
   `butchies-bootreport.timer`. Nach acht Sekunden und danach regelmäßig
   wird `Bootreport-<Boot-ID>.txt` mit Hardwareinventar, fehlender
   Kernel-Firmware, PCIe/NVMe-, WLAN- und Startinformationen erzeugt.
   Windows-lesbare Kopie: `NVMEDATEN/Bootreports/`; Systemkopie:
   `/root/Butchies-Recovery/Bootreports/`.
4. Der Windows-Stick-Ersteller wertet eine fehlgeschlagene `NVMEDATEN`-
   Partition jetzt als unvollständigen Stick aus. `make-stick.sh` legt
   diese Partition auch ohne zusätzlichen Datenordner an.
5. GitHub Actions baut erst das Linux-ISO, prüft enthaltenes Programm,
   Bootreport-Timer und Firmware-Prüfsumme, dann den Windows-Installer.
   Dieser enthält das ISO in `boot/` und prüft dessen SHA-256 vor der
   Offline-Verwendung. Release `linux-preview` kommt vom Entwicklungszweig;
   `aktuell` bleibt dem Main-Zweig vorbehalten. Fehlgeschlagene Builds
   veröffentlichen nichts.

## Nachweise und derzeitige Grenzen

- Quelltextprüfung: `git diff --check`, `bash -n` für Build- und
  Stick-Skripte, Python-Syntaxprüfung für `butchies-hardware`, YAML-Struktur.
- Erster Release-Lauf `46134175193` scheiterte bereits an einem fehlenden
  Ausführungsrecht von `linux-boot/build.sh`. Das Recht ist im Folgecommit
  korrigiert. Folgelauf `46134427019` verifizierte bereits beide Dell-Pakete
  samt innerer MUP-Datei, stoppte aber wegen fehlendem `python3` im
  Docker-Buildcontainer; die Paketliste wurde ergänzt. Den jeweils neuesten
  Folgelauf und seine Artefakte separat prüfen.
- Es gibt aus dieser Änderung noch **keinen bestätigten USB-Start auf dem
  Nutzergerät** und keine erfolgreiche Flashprobe. Ein altes ISO/alter Stick
  bekommt die neuen Reports nicht rückwirkend; er muss mit einem
  tatsächlich gebauten neuen Release neu geschrieben werden.
- Bei `144d:a808` ohne NVMe-Admin-Schnittstelle kann die Dell-Nutzlast
  **nicht** per Standard-NVMe-Befehl übertragen werden. Keinen Erfolg
  anzeigen und kein Reset-/Format-/Sanitize-Verfahren als Ersatz behaupten.

## Nächste Arbeitsschritte in Reihenfolge

1. GitHub-Workflow bis zum Ende beobachten; Linux- und Windows-Job,
   Release-Dateien, Dateigrößen und SHA-256 prüfen. Jeden konkreten Fehler
   anhand der Joblogs beheben und neu bauen. Build ohne Erfolgsnachweis
   nicht als fertigen Release melden.
2. Neues ISO auf Stick schreiben, `NVMEDATEN/Bootreports` unter Windows
   auf tatsächliche Dateien prüfen, neueste TXT hier als Hardwarebeleg
   auswerten. Wenn leer: Mountstatus, Timer, Dienstjournal und Bootmedium-
   Vergleich im Skript anhand realer Logs korrigieren.
3. Aus dem Report das konkrete PCIe-Gerät, Kernelversion, Treiberbindung,
   fehlende WLAN-Firmware und den Startfehler prüfen; nur belegte Treiber
   und passende Kernel-Firmware ergänzen.
4. Für PM981a-Notmodus nur einen nachweislich passenden Samsung/Dell-
   Recovery-Loader mit nachvollziehbarer Herkunft und Board-/NAND-/SED-
   Kompatibilität untersuchen. Ohne solchen Loader bleibt der Flashweg
   gesperrt; externe Laboranalyse ist dann der dokumentierte Weg.
5. Nach erfolgreichem USB-Test und überprüfter Firmware-/Datenpartition
   Release über den geprüften Workflow erstellen; PR #1 erst nach
   Auflösung seiner Merge-Konflikte und Bewertung der Main-Updatefolgen
   integrieren.

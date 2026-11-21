# Ablaufplan zur Weiterentwicklung durch andere KI-Agenten

**Zweck:** Eine andere KI soll das Projekt ohne Verlust von Produktanforderungen sicher fortsetzen können. Dieser Plan ist eine Arbeitsanweisung, keine Behauptung fertiger Funktionen.

## 0. Bestandsaufnahme
- GitHub-Repository `Butchies/Butchies-nvme-recovery-studio-ai` öffnen, Default-Branch `main`.
- `AGENTS.md`, Master-Spezifikation, Architektur, Sicherheitskonzept und aktuellen Code lesen.
- Vor jeder Änderung tatsächlichen Repository-Zustand gegen Dokumentation prüfen.
- Kein ungeprüftes Überschreiben, keine behaupteten Builds oder Hardwaretests.

## 1. Modul- und Schnittstellenplan

### Geräteinventar
- Linux: sysfs, libnvme/nvme-cli, PCIe-Topologie, Controller- und Namespace-IDs, SMART, Error-Logs.
- Windows: USB-PnP-Topologie, Bridge-ID und nachweislich externe NVMe-Ziele; interne Laufwerke vollständig aus Zielauswahl und Aktionspfad ausschließen.
- Datenmodell: Geräte-Fingerprint, Bus, Controller-IDs, Firmwarestand, Gerätepfad, System-/Bootschutz, Diagnosefähigkeiten.
- Unbekannte oder widersprüchliche Identität als nicht beschreibbar kennzeichnen.

### Diagnose
- Rohdaten strikt von KI-Interpretationen trennen.
- SMART/Health, NVMe Identify, Firmware-Slots, Error Information Log und PCIe-Status erfassen.
- Fehlercodes mit Quellen und Confidence/Unsicherheit interpretieren.
- Keine Firmwareänderung im Diagnosepfad.

### KI und Lernspeicher
- Lokales Modell optional einbinden; Offlinebetrieb ohne Cloud voraussetzen.
- Werkzeuge als explizite, protokollierte APIs anbieten; KI nicht direkt mit privilegiertem Shell-/Raw-Device-Zugriff ausstatten.
- SQLite-Fallakten: Gerätedaten, Beobachtungen, Hypothesen, Quellen, Versuche, Freigaben, Ergebnisse.
- Lernvorschläge zunächst als unbestätigt speichern; bestätigte Erkenntnisse versionieren, mit Herkunft und Rücknahmeoption.
- Keine automatische Selbständerung sicherheitskritischer Regeln oder Flash-Executor-Privilegien.

### Firmware und Recovery
- Manifest je Firmware: Hersteller, Lizenz/Redistribution, SHA-256, Signatur, Controller, Board, NAND, Revision, kompatible Geräte, Verfahren, Herkunft.
- Signatur-/Kompatibilitätsprüfung unabhängig vom Sprachmodell implementieren.
- Privilegierter Executor separat, gerätegebundene Freigabe, Audit-Log, Abbruch-/Timeout-Regeln.
- Custom-Firmware-Modus: Hardwaredokumentation, Codegenerierung, Build, statische Analyse, Sandbox, dokumentierter Programmierweg, manuelle Freigabe.
- Signaturgesperrte und unbekannte Controller nicht als universell reparierbar darstellen.

### Benutzeroberfläche
- Qt 6/QML, dunkles Design.
- Tabs: Übersicht, Geräte, Diagnose, KI-Live-Monitor, Firmware, Recovery, Fallakten, Netzwerk, Einstellungen.
- KI-Live-Monitor: Rohmessungen, Werkzeugaufrufe, Hypothesen, Belege, Entscheidungen, Code-Diffs und Buildprotokolle.
- Einfach-/Expertenmodus; Simulation immer deutlich kennzeichnen.

### Packaging
- Windows: Installer mit ausschließlich USB-NVMe-Zugriff.
- Linux: UEFI-USB-Image, Vollbild-Kiosk, WLAN über NetworkManager, Offline-Modell, verschlüsselte persistente Daten.
- Offline-Ressourcen soweit lizenzrechtlich möglich; Aktualisierung signiert und nachvollziehbar.
- Keine PDF-Handbücher mit Bildern vor fertiger Software.

## 2. Reihenfolge der Arbeitspakete
| ID | Aufgabe | Abhängigkeit | Abnahme |
|---|---|---|---|
| A01 | Einheitliches Geräte-/Diagnoseschema | keine | dokumentierte Modelle |
| A02 | Linux read-only Inventory erweitern | A01 | nachvollziehbare sysfs/NVMe-Ausgaben |
| A03 | Windows USB-only Discovery | A01 | interne Geräte ausgeschlossen |
| A04 | SMART/Identify/Error Logs | A02/A03 | rohe Messdaten + Fehlerbehandlung |
| A05 | SQLite-Fallakten und Export | A01 | Ereignisse reproduzierbar |
| A06 | Qt-UI und Live-Monitor | A04/A05 | echte Daten, keine erfundenen Logs |
| A07 | Firmwaremanifest + Validierung | A01 | fail-closed bei Unbekanntem |
| A08 | Offline-KI und kontrollierte Tool-API | A04/A05 | kein direkter Flashzugriff |
| A09 | Recovery-Executor mit Freigaben | A07/A08 | Zielbindung, Audit, Sperren |
| A10 | Linux Boot USB + WLAN | A02/A06 | Boot- und Netzwerktests auf Zielhardware |
| A11 | Windows Installer | A03/A06 | installierbarer Build |
| A12 | Experimentelle Firmwareentwicklung | A07/A09 | Code/Prüfung/Freigabe, keine Universalversprechen |
| A13 | Offline-Pakete, Updatekanal | A08/A10/A11 | Signaturen, Lizenzprüfung |
| A14 | End-to-End-Validierung | alle | echte Testergebnisse dokumentiert |
| A15 | Bebilderte DE/EN-Handbücher | A14 | echte Screenshots beider Editionen |

## 3. Fortsetzungsprotokoll
Bei jeder Übergabe die Datei `docs/ENTWICKLUNGSSTAND_DE.md` aktualisieren:
- Letzte abgeschlossene Arbeitspakete
- Geänderte Dateien/Commits
- Was real funktioniert, was nur vorbereitet ist
- Tests durchgeführt/nicht durchgeführt und Ergebnis
- Bekannte Risiken und Blocker
- Konkrete nächste Aufgabe mit Akzeptanzkriterien

## 4. Verbesserungsprozess
- Änderungen an Architektur und Datenmodellen in kurzen ADRs unter `docs/decisions/` festhalten.
- Sicherheitsinvarianten dürfen nicht durch Modellvorschläge gelockert werden.
- Messbare Verbesserungen bevorzugen: höhere Erkennungsrate, weniger Fehlklassifikation, bessere Offlineabdeckung, nachvollziehbarere Entscheidungen.
- Regressionen dokumentieren und Änderungen bei Bedarf zurücknehmen.
- Firmware-Recovery erst nach bestätigter Hardware-/Controllerkompatibilität.

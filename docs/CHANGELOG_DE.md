# Änderungsprotokoll

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

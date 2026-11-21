# Hardware-Diagnoseassistent – Produktentwurf

## Ziel
Ein geführter Bereich soll feststellen helfen, ob die Ursache einer nicht erreichbaren oder instabilen NVMe-SSD eher beim Rechner, USB-Gehäuse, Kabel, Anschluss, Treiber, der Stromversorgung oder der SSD selbst liegt. Die KI soll beobachtete Daten mit dem ausgewählten Gerät verknüpfen und Unsicherheit sichtbar machen.

## Einstieg
- Schaltfläche „Rechner und Verbindung prüfen“ auf Dashboard und SSD-Detailseite.
- Auswahl der betroffenen externen USB-NVMe-SSD unter Windows 10/11 beziehungsweise der PCIe-NVMe-SSD im Linux-Bootsystem.
- Kurze Frage nach dem Symptom: nicht erkannt, Verbindung bricht ab, ungewöhnlich langsam, Fehler bei Zugriffen, sehr heiß, unbekannter Fehler.
- Wahl zwischen „Schnellübersicht“ und „Detaillierte Analyse“.

## Analyseansicht
- Rechnerprofil mit CPU, RAM, Mainboard, Betriebssystem und relevanten Controllern.
- Anschlusskette sichtbar darstellen: Rechner → Hostcontroller → Port → Kabel/Gehäuse/USB-Bridge → SSD; unter Linux PCIe-Pfad statt USB-Kette.
- Je Abschnitt erkennbare Daten, bekannte Einschränkungen und fehlende Informationen anzeigen.
- Tatsächliche Verbindungsgeschwindigkeit von theoretisch möglicher Geschwindigkeit unterscheiden.
- Host- und Gerätetemperaturen sowie relevante Ereignisse anzeigen, soweit verfügbar.
- Wenn ein Gerät nicht identifizierbar ist, den Zustand „unbekannt“ ausdrücklich anzeigen.

## KI-Einschätzung
- Beobachtungen, mögliche Ursachen und Empfehlungen in getrennten Feldern darstellen.
- Ursachen nach Plausibilität ordnen, ohne unbelegte Prozentwerte zu erfinden.
- Erklären, warum eine andere USB-Buchse, ein anderes Kabel, ein anderes Gehäuse oder direkter PCIe-Zugriff möglicherweise hilfreich ist.
- Bei fehlender Hardwaretelemetrie auf die Grenzen der Analyse hinweisen.
- Diagnosen über mehrere Sitzungen vergleichbar machen.

## Bedienhilfen
- Grafische Anschlussübersicht mit markierten Auffälligkeiten.
- Verständliche Erklärungen zu USB 2.0/3.x, UASP, PCIe-Generation und Linkbreite.
- „Was bedeutet das?“ direkt an jedem auffälligen Eintrag.
- Ergebnis an die SSD-Fallakte und den KI-Live-Monitor übergeben.

## Schutz
- Rechneranalyse ist standardmäßig lesend.
- Keine eigenmächtigen BIOS-, Treiber-, Energie- oder Firmwareänderungen.
- Unter Windows bleiben interne Laufwerke von SSD-Recovery-Aktionen ausgeschlossen.
- Keine belastenden Schreib- oder Stresstests auf verdächtigen SSDs.

## Noch gewünschte Funktionen
Geführter Assistent, grafische Anschlusskette, symptombezogene KI-Einordnung und Fallaktenvergleich.

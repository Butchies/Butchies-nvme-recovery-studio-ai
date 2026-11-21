# Entwicklungsanweisungen für KI-Coding-Agenten

Dieses Repository enthält **NVMe Recovery Studio AI**. Lies vor jeder Änderung:
1. `README.md`
2. `docs/MASTER_SPECIFICATION_DE.md`
3. `docs/ARCHITEKTUR_DE.md`
4. `docs/SICHERHEIT_DE.md`
5. `docs/AI_ENTWICKLUNGSPLAN_DE.md`
6. `docs/ENTWICKLUNGSSTAND_DE.md`

## Unveränderliche Produktregeln
- Windows 11 Edition: **nur externe USB-NVMe-Geräte**. Keine internen Datenträger als Ziele, auch nicht für experimentelle Aktionen.
- Linux Boot Edition: direktes PCIe/NVMe, bootfähiges USB-Medium, Vollbild-Kiosk, WLAN, Offline-Betrieb. Bootmedium und Systemdatenträger nie beschreiben.
- Standardmäßig nur lesende Diagnosen. Firmware-Schreibzugriffe ausschließlich mit geprüfter Kompatibilität, nachgewiesenem Programmierweg, eindeutigem Ziel und ausdrücklicher, separater Benutzerfreigabe.
- KI darf neue Diagnosemethoden und experimentelle Firmware entwickeln, aber kein selbständiges, unbestätigtes Flashen. Unbekannte Kompatibilität bedeutet blockieren.
- Lokale KI, Fallgedächtnis, nachvollziehbarer Live-Monitor. **Keine erfundenen Gedankenprotokolle**: Messwerte, Werkzeugausgaben, Hypothesen, Quellen und begründete Entscheidungen protokollieren.
- Proprietäre Firmware und Tools nur mit geklärten Rechten integrieren. Keine nicht vorhandenen Binärdateien oder Hardwarefunktionen behaupten.
- Handbücher DE/EN mit echten Screenshots erst nach Fertigstellung der Software erstellen.

## Arbeitsablauf für jeden Agenten
1. Repository und Branch prüfen; bestehende Implementierung lesen.
2. Eine klar abgegrenzte Aufgabe aus `docs/AI_ENTWICKLUNGSPLAN_DE.md` auswählen.
3. Betroffene Schnittstellen und Sicherheitsinvarianten dokumentieren.
4. Implementierung klein und modular halten; neue Abhängigkeiten und Lizenz erfassen.
5. Soweit möglich automatisierte Unit-/Integrationstests hinzufügen; Hardwaretests niemals vortäuschen.
6. `docs/ENTWICKLUNGSSTAND_DE.md` und `docs/CHANGELOG_DE.md` aktualisieren.
7. Änderungen auf eigenem Feature-Branch mit nachvollziehbaren Commits einreichen, sofern verfügbar. Keine fremden Änderungen überschreiben.
8. Abschlussbericht: Dateien, implementierte Funktionen, Teststatus, Risiken, offene Punkte, nächster Schritt.

## Definition of Done
Eine Aufgabe ist erst fertig, wenn ihr Code eingecheckt, Sicherheitsanforderungen berücksichtigt, Schnittstellen dokumentiert und Prüfstatus ehrlich festgehalten sind. Ein Entwurf oder UI-Mockup ist kein fertiger Hardware-Recovery-Build.

## Priorität
1. Geräteidentität und Schutzmechanismen
2. Lesende Hardwarediagnose
3. Lokale Fallakten und nachvollziehbare UI
4. Firmwarekatalog und Kompatibilitätsprüfung
5. Freigabegesteuerte Recovery
6. Experimentelle KI-Firmwareentwicklung
7. Installationspakete und bootfähige Linux-Edition
8. Abschließende bebilderte Handbücher

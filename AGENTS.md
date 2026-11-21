# NVMe Recovery Studio AI – Projektentwurf für die weiterentwickelnde KI

Dieses Dokument übergibt ausschließlich unsere Produktidee und Anforderungen. Die andere KI entscheidet selbst über Programmierung, Umsetzung, Qualitätssicherung und ihren Arbeitsablauf. Hier werden **keine Tests vorgeschrieben oder durchgeführt**.

## Ziel
Eine eigenständige, möglichst offlinefähige KI-Reparatursoftware für M.2-NVMe-SSDs mit zwei Editionen.

## Edition 1: Windows
- Windows 10 und 11 mit moderner dunkler Oberfläche.
- Ausschließlich **extern per USB angeschlossene NVMe-SSDs** bearbeiten.
- USB-Bridge erkennen und anzeigen, welche Diagnose- und Firmwarefunktionen durchgereicht werden.
- Wenn eine Bridge nicht geeignet ist, Ursache und belegte Alternativen anzeigen.
- Interne Laufwerke niemals als Reparaturziel anbieten.

## Edition 2: Linux Boot USB
- Bootfähiger USB-Stick, der unmittelbar die Recovery-Anwendung im Vollbild startet.
- Direkter PCIe-Zugriff auf intern verbaute NVMe-SSDs.
- WLAN zum Abruf fehlender Firmware, Controllerinformationen und Updates.
- Auch offline mit lokal mitgelieferten Werkzeugen, Datenbanken, Hilfen und KI-Modell nutzbar.
- Bootmedium und andere nicht ausgewählte Datenträger schützen.

## KI-Funktionen
- Controller, Firmware, NAND und Fehlerzustand soweit technisch erkennbar analysieren.
- Geeignete Firmware und Recovery-Strategien suchen und zuordnen.
- Reparaturfälle lokal speichern, aus bestätigten Ergebnissen lernen und Wissen erweitern.
- Bei fehlender Original-Firmware experimentell eigene Diagnose-, Boot- oder Controller-Firmware entwickeln können, soweit die Hardware dies technisch zulässt.
- Vor tatsächlichen Firmware-Schreibvorgängen immer verständliche Risiken anzeigen und die ausdrückliche Freigabe des Benutzers einholen.

## KI-Live-Monitor
- Live anzeigen, welche Daten gelesen werden, welche Hypothesen entstehen, welche Entscheidung vorgeschlagen wird und warum.
- Technische Messwerte, Fehlercodes, Quellen, Protokolle, Codeänderungen und Firmwareentwicklungsfortschritt sichtbar machen.
- Einfacher und Expertenmodus.
- Keine angebliche Einsicht in verborgene Modellgedanken; stattdessen nachvollziehbare Analyse- und Entscheidungsprotokolle.

## Gestaltung
- Professionelle, aufgeräumte dunkle Oberfläche.
- Bereiche für Dashboard, Geräte, Diagnose, KI-Live-Monitor, Firmware, Recovery, Fallakten, WLAN und Einstellungen.
- Windows-Installer und Linux-Bootpaket als eigenständige Ausgaben.

## Handbücher
Deutsche und englische bebilderte PDF-Anleitungen erst erstellen, wenn die Software fertig programmiert ist, mit echten Screenshots.

## Übergabe
Die übrigen Projektdokumente erläutern den Entwurf. **Alle Entscheidungen zur konkreten Implementierung und zu Prüfungen liegen bei der anderen KI.** Keine Implementierung oder Funktion als fertig darstellen, solange sie nicht tatsächlich existiert.

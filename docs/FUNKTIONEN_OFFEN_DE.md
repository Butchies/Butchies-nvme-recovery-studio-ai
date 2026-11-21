# NVMe Recovery Studio AI – Funktionswünsche und noch offene Umsetzung

Dies ist eine reine Produkt-Wunschliste für die andere KI. Keine Testpläne, keine Implementierungsvorgaben.

| Bereich | Gewünschte Funktion | Stand des Entwurfs |
|---|---|---|
| Windows | Nur externe USB-NVMe-Geräte erkennen und bearbeiten | Noch zu programmieren |
| USB | Bridge-Chips erkennen und Einschränkungen erklären | Noch zu programmieren |
| Linux | Bootfähiges USB-Recovery-System mit Vollbild-App | Noch zu programmieren |
| Linux | Direkte PCIe-/NVMe-Geräteerkennung | Erste einfache Inventarisierung vorhanden, Erweiterung offen |
| Netzwerk | WLAN, Online-Downloads, Offline-Fallback | Noch zu programmieren |
| Diagnose | SMART, Identify, Fehlerlogs, Firmware-Slots | Noch zu programmieren |
| Controllerwissen | Hersteller-, Controller- und NAND-Profile | Noch zu programmieren |
| KI | Lokales KI-Modell und verständliche Diagnosen | Noch zu programmieren |
| KI-Gedächtnis | Lokale Fälle und bestätigte Erkenntnisse | Erste SQLite-Grundlage vorhanden, Erweiterung offen |
| KI-Live-Monitor | Fakten, Hypothesen, Quellen, Entscheidungen | Noch zu programmieren |
| Firmware | Katalog, Herkunft, Kompatibilität, Download | Noch zu programmieren |
| Recovery | Datenerhaltende Maßnahmen und freigegebene Reparatur | Noch zu programmieren |
| KI-Labor | Eigene experimentelle Diagnose-/Controller-Firmware | Noch zu programmieren |
| Oberfläche | Dashboard und alle Hauptbereiche | Noch zu programmieren |
| Installer | Windows-Installationspaket | Noch zu programmieren |
| Linux-Ausgabe | Bootfähiges Recovery-USB-Image | Noch zu programmieren |
| Dokumentation | Bebilderte DE/EN-PDFs mit echten Screenshots | Erst nach Softwarefertigstellung |

## Festgelegte Leitlinien
- Windows darf keine internen SSDs als Reparaturziele anbieten.
- Linux-Bootmedium und nicht ausgewählte Datenträger schützen.
- Offlinebetrieb ist ein Kernwunsch.
- Vor tatsächlichem Flashen ist eine ausdrückliche Freigabe notwendig.
- KI-Firmwareentwicklung ist experimentell und durch reale Hardwaregrenzen eingeschränkt.
- Die andere KI übernimmt sämtliche Entscheidungen zur Programmierung, Verbesserung und Qualitätssicherung.

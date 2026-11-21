# Controller- und SSD-Wissensdatenbank – Produktentwurf

Dieses Dokument beschreibt die gewünschten Inhalte und Funktionen, nicht die technische Umsetzung.

## Ziel
NVMe Recovery Studio AI soll möglichst viele NVMe-SSDs anhand von Modell, Controller, NAND, Board-Revision und Firmwarestand einordnen können. Die Anwendung soll Unsicherheiten ausdrücklich anzeigen und unbekannte Hardware nicht fälschlich als kompatibel behandeln.

## Gewünschte Controller- und Herstellerbereiche
- Phison
- Silicon Motion
- Maxio
- Innogrit
- Realtek
- Samsung
- Western Digital / SanDisk
- Kioxia / Toshiba
- SK hynix
- Micron
- Weitere Controllerfamilien sollen ergänzbar sein.

Die Nennung eines Herstellers bedeutet nicht, dass dessen Controller bereits unterstützt wird.

## Informationen je SSD
- Hersteller, Modell, Modellvarianten, Kapazität und Seriennummer.
- Controllerhersteller, genaue Controllerkennung, Revision und Firmwarestand.
- NAND-Hersteller, NAND-Typ, Dies/Packages und bekannte Kompatibilitäten, soweit tatsächlich ermittelbar.
- PCIe-Generation, NVMe-Version, Namespace-Informationen und unterstützte Funktionen.
- USB-Bridge-Hersteller und Modell bei externen SSDs.
- Bekannte Fehlerbilder, Symptome und mögliche Ursachen.
- Verfügbare Diagnosewege, Recovery-Optionen und technische Grenzen.
- Dokumentierte Firmwarestände, Bezugsquellen, Versionsunterschiede und Kompatibilitätsbedingungen.
- Quellen, Herkunft und Vertrauenswürdigkeit der Informationen.

## Gewünschte Bedienung
- Suchfeld für Modell, Controllerkennung, Firmware und Fehlercode.
- SSD-Profil mit Fakten, möglichen Zuordnungen und unbekannten Feldern.
- Vergleich mehrerer Geräte- und Firmwarevarianten.
- Warnung bei ähnlichen Modellnamen mit unterschiedlichem Controller oder NAND.
- Verknüpfung von Wissensartikeln mit lokalen Reparaturfällen.
- Lokale Datenbank auch ohne Internet nutzbar; optionale Aktualisierung bei Netzwerkzugang.

## KI-Erweiterung
- Neue Erkenntnisse aus Diagnose- und Reparaturfällen als Vorschläge aufnehmen.
- Beobachtung, Vermutung und bestätigtes Wissen klar unterscheiden.
- Widersprüchliche Quellen und unbestätigte Kompatibilität sichtbar machen.
- Wissen erweitern, ohne aus Vermutungen automatisch sichere Flashfreigaben abzuleiten.

## Noch offen
Vollständige Controllerprofile, NAND-Zuordnung, Firmwarewissen, Suchansichten, Quellenverwaltung und KI-Verknüpfung müssen durch die andere KI umgesetzt werden.

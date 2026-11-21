# NVMe-Subsysteme, Controller und Namespace-Zuordnung – Produktentwurf

## Ziel
Die Anwendung soll zwischen einer physischen SSD, ihrem NVMe-Subsystem, möglichen Controllern und den sichtbaren Namespaces unterscheiden. So sollen Mehrfachanzeigen und falsche Gerätezuordnungen vermieden werden.

## Topologie
- Physisches Gerät und NVMe-Subsystem getrennt darstellen.
- Controllerkennungen und verfügbare Beziehungen zu Namespaces anzeigen.
- Private und gemeinsam zugängliche Namespaces erklären.
- Erkennbare Pfade und Transportinformationen mit eigener Quelle kennzeichnen.
- Unterschied zwischen SSD, Controller, Namespace, Partition und Laufwerksbuchstabe verdeutlichen.

## Geräteansicht
- Hierarchische Grafik: Gerät → Subsystem → Controller → Namespace → Partition/Dateisystem.
- Bei mehreren Controllern keine zusätzliche physische SSD erfinden.
- Geräteidentität mit bestehenden Fallakten und Hardwareprofilen verknüpfen.
- Veränderungen der sichtbaren Topologie über Sitzungen markieren.
- Bei unvollständigen Angaben eine teilweise bekannte Struktur zeigen.

## Datenrettung
- Vor der Auswahl des Rettungsziels eindeutig anzeigen, welcher Namespace zu welchem Gerät gehört.
- Mehrfach erreichbare Namespaces nicht als unabhängige Datenkopien darstellen.
- Keine Namespace-Erstellung, -Löschung oder -Neu-Zuordnung aus der normalen Übersicht.
- Vorhandene Abbilder der korrekten Namespace-Identität zuordnen.

## KI-Unterstützung
- Abweichende Kapazitäten und doppelte Geräteansichten mit der Topologie vergleichen.
- Fehlende Pfade nicht als Datenverlust behaupten.
- Unsicherheit bei USB-Bridge-Abstraktion sichtbar machen.

## Noch gewünschte Funktionen
Subsystem-Baum, Controller-Namespace-Mapping, Fallaktenzuordnung und Verwechslungsschutz.

## Quelle
NVM Express: NVMe Namespaces: https://nvmexpress.org/resource/nvme-namespaces/

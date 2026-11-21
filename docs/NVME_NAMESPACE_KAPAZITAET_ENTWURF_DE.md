# NVMe-Namespaces und Kapazitätsabweichungen – Produktentwurf

## Ziel
Die Anwendung soll erklären, warum ein Controller, ein Namespace, eine Partition und ein sichtbares Betriebssystemlaufwerk unterschiedliche Größen und Kennungen haben können.

## Geräteansicht
- NVMe-Controller, erkannte Namespaces und deren Identifikatoren getrennt darstellen.
- Namespace-Kapazität, genutzte Kapazität, logische Blockgröße und Metadaten, soweit verfügbar.
- Physische SSD-Kapazität, Namespacegröße und Partitionierung gegenüberstellen.
- Fehlende oder veränderte Namespaces als Beobachtung mit Zeitstempel dokumentieren.
- USB-Bridge-Einschränkungen bei der Sichtbarkeit von Namespaces kennzeichnen.

## KI-Einordnung
- Plausible Gründe für abweichende Kapazitäten erklären.
- Unterschiede zwischen normaler Reservierung, geänderter Konfiguration und möglichem Fehlerbild darstellen.
- Frühere Namespace-Aufnahmen mit dem aktuellen Zustand vergleichen.
- Keine automatische Neuformatierung oder Namespace-Neuanlage vorschlagen, um vermeintlich fehlenden Platz zurückzuholen.
- Unbekannte Ursachen als solche markieren.

## Oberfläche
- Grafische Hierarchie Controller → Namespace → Partition → Dateisystem.
- Warnhinweise bei unerwarteten Größenänderungen.
- Erklärung von NSID und stabilen Identifikatoren, soweit vorhanden.
- Übergabe an Partitionsanalyse und Fallakte.

## Schutz
- Namespace-Management als potenziell datenverändernde Funktion klar vom lesenden Diagnosebereich trennen.
- Keine automatischen Namespace-Erstellungs-, Lösch- oder Formatierungsvorgänge.

## Noch gewünschte Funktionen
Namespace-Baum, Kapazitätsvergleich, Abweichungsanalyse und historische Zuordnung.

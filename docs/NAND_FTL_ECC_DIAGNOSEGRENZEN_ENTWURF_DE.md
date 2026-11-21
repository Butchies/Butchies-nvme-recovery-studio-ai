# NAND, FTL und ECC: Diagnosemöglichkeiten und Grenzen

## Ziel
NVMe Recovery Studio AI soll bei Controller- und Flashproblemen erklären, welche Informationen aus standardisierten NVMe-Daten tatsächlich hervorgehen und welche Details zu NAND, Fehlerkorrektur (ECC) und Flash Translation Layer (FTL) ohne Herstellerwissen unbekannt bleiben.

## Diagnoseansicht
- Physische NAND-Bausteine, Controller und logische Namespaces als unterschiedliche Ebenen darstellen.
- Sichtbare Informationen aus Identify, SMART, Error Logs und optionalen Herstellerlogs mit Quellen versehen.
- ECC-Fehlerstatistiken, Blockzustände und Mappinginformationen nur zeigen, wenn das konkrete Gerät sie verlässlich bereitstellt.
- Unbekannte NAND-Geometrie, Bad-Block-Listen oder FTL-Tabellen nicht aus Modellnamen erraten.
- Controller-Bootmodus und unzugängliche Nutzerdaten getrennt bewerten.

## KI-Erklärungen
- Verständlich erläutern, weshalb ein logisch adressierbarer NVMe-Block nicht zwingend einem festen NAND-Speicherort entspricht.
- Unkorrigierbare Lesefehler, Controllerfehler, Mappingprobleme und Transportfehler als unterschiedliche Hypothesen behandeln.
- Herstellerabhängige ECC-, Wear-Leveling- und FTL-Mechanismen mit Unsicherheitskennzeichnung erklären.
- Keine universelle Rekonstruktion von FTL oder NAND-Daten behaupten.
- Grenzen bei verschlüsselten Controllern und fehlenden Schlüsseln ausdrücklich nennen.

## Bezug zum experimentellen Firmwarelabor
- Ein selbst erzeugter Diagnose-Firmwareentwurf ist kein Nachweis, dass NAND-Daten korrekt interpretierbar sind.
- Unbekannte Initialisierung, Signaturen, ECC-Verfahren und FTL-Metadaten als harte Kompatibilitätsgrenzen kennzeichnen.
- Vor jeder möglichen Controlleränderung Datenrettung, bestehende Abbilder und Geräteidentität hervorheben.
- Keine automatisch generierten Firmwaredateien ohne gesonderte Benutzerfreigabe auf Hardware schreiben.

## Oberfläche
- Schichtenansicht: Host/Transport → NVMe-Controller → FTL/ECC → NAND.
- Pro Ebene: bekannte Fakten, mögliche Fehlerbilder, fehlende Informationen.
- Verknüpfung mit Bauteilfotos, Controllerwissen, Firmwarebibliothek und Fallakte.

## Noch gewünschte Funktionen
Schichtenmodell, FTL/ECC-Grenzen, Unsicherheitsanzeige und Verbindung zum KI-Firmwarelabor.

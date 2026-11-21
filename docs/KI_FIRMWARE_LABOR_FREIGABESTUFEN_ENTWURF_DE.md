# KI-Firmwarelabor: Freigabestufen und nachvollziehbare Entscheidungen

## Ziel
Das experimentelle KI-Firmwarelabor soll Diagnoseideen, Firmwareentwürfe und tatsächliche Geräteänderungen strikt voneinander trennen. Ein KI-Vorschlag darf niemals als bereits geprüfte oder sichere Firmware erscheinen.

## Stufen
1. **Analyse:** Controller, NAND, Boardrevision, Firmwarekennung, vorhandene Logs und Wissensquellen erfassen.
2. **Hypothese:** mögliche Ursache und erforderliche unbekannte Informationen auflisten.
3. **Entwurf:** gegebenenfalls ein controllerabhängiges Diagnose- oder Recovery-Firmwarekonzept mit klarer Kennzeichnung als experimentell darstellen.
4. **Kompatibilitätsbewertung:** belegte und fehlende Informationen zu Signaturen, Bootpfad, Speicherlayout, NAND, ECC und FTL offenlegen.
5. **Benutzerentscheidung:** mögliche Konsequenzen einschließlich endgültigem Datenverlust und Geräteausfall getrennt bestätigen lassen.
6. **Nachvollziehbarkeit:** gewählte Option, Quellen, bekannte Grenzen und Ergebnis in der Fallakte dokumentieren.

## Sicherheitsgrenzen
- Keine universelle Firmwaregenerierung versprechen.
- Ein Entwurf ist weder funktionsfähig noch kompatibel, nur weil die KI ihn erzeugt hat.
- Unbekannte Signaturen, Verschlüsselung, Controllerbefehle oder NAND-Parameter als Blocker kennzeichnen.
- Keine automatischen Schreibvorgänge durch den Live-Monitor, die Diagnose oder das Öffnen einer Fallakte.
- Geräteidentität und exakte Zielhardware vor jeder potenziellen Änderung erneut anzeigen.
- Original-Firmware, gesicherte Daten und Recovery-Abbilder als separate Ressourcen behandeln.

## Oberfläche
- Labor-Dashboard mit Stufe, Evidenz, offenen Punkten und Risikohinweisen.
- Gegenüberstellung von Original-Firmware, dokumentierten Recovery-Optionen und experimentellen Entwürfen.
- Klare Kennzeichnung von Simulation, Dokumentation und tatsächlichem Hardwareeingriff.
- Keine versteckte Umgehung der Freigabeschritte durch Expertenmodus.

## Noch gewünschte Funktionen
Stufenansicht, Kompatibilitätsblocker, eindeutige Freigaben und Fallaktenhistorie.

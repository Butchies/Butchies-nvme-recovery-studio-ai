# NVMe-Standardversionen und Funktionskompatibilität – Produktentwurf

## Ziel
Die Anwendung soll Funktionen nur dann als verfügbar anzeigen, wenn sie vom konkreten Controller und Anschluss tatsächlich unterstützt werden. Die NVMe-Spezifikation und das Alter eines Geräts dürfen nicht verwechselt werden.

## Funktionsübersicht
- Gemeldete NVMe-Version und relevante Controllerfähigkeiten, soweit lesbar.
- Unterstützte Logseiten und optionale Diagnosefunktionen.
- Unterschied zwischen Standardfunktion, optionaler Funktion und herstellerspezifischer Erweiterung.
- Einschränkungen durch USB-Bridge, Betriebssystem oder Treiber.
- „Nicht verfügbar“, „nicht unterstützt“ und „nicht überprüfbar“ getrennt darstellen.

## Benutzeroberfläche
- Kompatibilitätssteckbrief je SSD.
- Funktionsliste mit Erklärung und Datenquelle.
- Verweise zu Log-Explorer, Telemetrie, Firmware-Slots und Selbstdiagnose.
- Hinweis, wenn eine Funktion erst in neueren Spezifikationen beschrieben wurde, aber nicht automatisch auf jedem neueren Gerät vorhanden ist.

## KI-Unterstützung
- Empfehlungen an tatsächlich belegte Fähigkeiten anpassen.
- Herstellerangaben und Gerätemeldungen getrennt behandeln.
- Fehlende Befehlsunterstützung nicht als Hardwaredefekt ausgeben.
- Unbekannte Merkmale nicht als unterstützt voraussetzen.

## Noch gewünschte Funktionen
NVMe-Funktionsmatrix, Versionsvergleich und verständliche Kompatibilitätshinweise.

## Referenz
Offizielle NVM Express Spezifikationsübersicht:
https://nvmexpress.org/specification/nvm-express-base-specification/

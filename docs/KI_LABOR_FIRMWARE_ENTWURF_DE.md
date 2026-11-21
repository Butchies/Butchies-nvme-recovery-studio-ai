# KI-Labor für experimentelle Firmwareentwicklung – Produktentwurf

## Idee
Wenn keine passende Hersteller-Firmware oder dokumentierte Recovery-Methode verfügbar ist, soll NVMe Recovery Studio AI einen getrennten experimentellen Entwicklungsbereich anbieten. Die KI darf dort untersuchen, ob eigene Diagnose-, Boot-, Recovery- oder Controller-Firmware für die konkrete Hardware überhaupt realistisch ist.

## Gewünschte Inhalte des KI-Labors
- Identität und technische Informationen des ausgewählten Controllers.
- Übersicht vorhandener und fehlender Controller-/NAND-Dokumentation.
- Bekannte Firmwareformate, Schnittstellen und Hardwarebeschränkungen.
- KI-Vorschläge für Diagnose- und Recovery-Code.
- Sichtbarer Quelltext, Versionen, Änderungen und Begründungen.
- Fortschrittsanzeige für experimentelle Entwicklungsprojekte.
- Dokumentation offener Fragen, Unsicherheiten und möglicher Risiken.
- Trennung zwischen theoretischem Entwurf, erzeugtem Code und tatsächlich auf Hardware einsetzbarer Firmware.

## Wichtige Grenzen
- Viele SSD-Controller sind proprietär und verwenden signierte Firmware.
- NAND-Ansteuerung, ECC, Flash Translation Layer, Wear Leveling und Bootabläufe sind oft nicht dokumentiert.
- Eine eigene Firmware ist deshalb nicht für jeden Controller möglich.
- KI-generierte Firmware darf niemals automatisch auf ein Gerät geschrieben werden.
- Vor jeder tatsächlich schreibenden Firmwareaktion muss der Benutzer ausdrücklich zustimmen.

## Gewünschte Verbindung zum Live-Monitor
- Erklärte Ziele, bekannte Fakten und Hypothesen.
- Entstehende Quelltexte und Änderungen.
- Fehlende technische Informationen und Grenzen.
- Nachvollziehbare Ereignisse statt angeblicher Einsicht in versteckte Modellgedanken.

## Noch offen
Der gesamte KI-Laborbereich, seine Benutzeroberfläche und die tatsächlichen Entwicklungsfunktionen.

# NVMe Recovery Studio AI – geplanter Bedienablauf

Dieses Dokument beschreibt ausschließlich den gewünschten Ablauf aus Benutzersicht. Die andere KI entscheidet über technische Umsetzung und Prüfungen.

## 1. Programmstart
- Windows: Anwendung starten, nur extern angeschlossene USB-NVMe-Geräte anzeigen.
- Linux: vom Recovery-USB-Stick booten, direkt in die Vollbildoberfläche wechseln.
- Startseite zeigt Verbindungsstatus, Offline-/Online-Modus, WLAN und verfügbare Geräte.
- Ohne erkanntes Gerät erscheint eine verständliche Hilfe zur Verbindung.

## 2. Gerät auswählen
- Übersicht je SSD mit Hersteller, Modell, Kapazität, Seriennummer (bei Bedarf maskiert), Firmwarestand, Controller und Verbindungstyp.
- USB-Bridge, soweit erkennbar, separat anzeigen.
- System- und Bootlaufwerke bleiben geschützt.
- Warnung bei unvollständiger Geräteidentifikation.

## 3. Diagnose starten
- KI zeigt laufend, welche Informationen vorliegen und welche noch fehlen.
- Diagnoseansicht enthält Gesundheitszustand, SMART-Werte, Fehlerprotokolle, Controller-/NAND-Informationen und Firmware-Slots, soweit verfügbar.
- Klare Unterscheidung zwischen bestätigten Fakten, Verdacht und unbekannten Angaben.
- Bei fehlenden Zugriffsmöglichkeiten erklären, ob die USB-Bridge die Ursache sein könnte.

## 4. Ergebnis und Lösungsvorschläge
- Übersicht der erkannten Symptome und möglichen Ursachen.
- Reparaturvorschläge nach Eignung und Risiko.
- Zuerst datenerhaltende Möglichkeiten anzeigen.
- Für jede vorgeschlagene Maßnahme erklären, welche Daten betroffen sein könnten und was technisch nicht garantiert werden kann.

## 5. Firmwareverwaltung
- Vorhandene Firmware, bekannte kompatible Versionen und Herkunft anzeigen.
- Fehlende Ressourcen bei bestehender Internetverbindung optional nachladen.
- Offline verfügbare Ressourcen sichtbar kennzeichnen.
- Wenn keine passende Firmware existiert, experimentelle KI-Entwicklung nur als getrennte Option anbieten.

## 6. Experimentelle KI-Firmwareentwicklung
- KI zeigt, welche Controllerinformationen und technischen Unterlagen vorhanden sind.
- Entwicklungsansicht für mögliche eigene Diagnose-, Recovery- oder Controller-Firmware.
- Quelltext, Änderungen, Fortschritt, bekannte Grenzen und Unsicherheiten darstellen.
- Keine automatische Firmwareinstallation ohne ausdrückliche Benutzerentscheidung.
- Wenn ein Controller durch Signaturen oder unbekannte Hardware blockiert ist, dies klar mitteilen.

## 7. KI-Live-Monitor
- Immer erreichbar, auch während Diagnose und Firmwareentwicklung.
- Zeitleiste für Messwerte, Werkzeugergebnisse, Hypothesen und vorgeschlagene Entscheidungen.
- Umschaltung zwischen einfacher Erklärung und Expertenansicht.
- Exportierbare, lokal gespeicherte Analyse- und Fallprotokolle.
- Keine Darstellung angeblich ausgelesener verborgener Modellgedanken.

## 8. Abschluss
- Ergebnisübersicht mit Gerät, Diagnose, vorgeschlagenen und tatsächlich ausgeführten Aktionen.
- Fall in der lokalen Datenbank speichern.
- Neue bestätigte Erkenntnisse für spätere Diagnosen nutzbar machen.
- Keine Reparaturerfolge behaupten, wenn das Ergebnis unbekannt ist.

## 9. Spätere Dokumentation
Bebilderte PDF-Handbücher auf Deutsch und Englisch erst nach Fertigstellung der Software mit echten Screenshots.

# NVMe-Befehlswirkungen und Sicherheitskennzeichnung – Produktentwurf

## Ziel
Die Expertenansicht soll NVMe-Befehle nach ihren möglichen Auswirkungen auf Controllerzustand, Namespaces und Nutzerdaten einordnen. Das Anzeigen eines Befehls ist niemals eine Ausführungsfreigabe.

## Fähigkeits- und Wirkungsübersicht
- Vom Gerät gemeldete Informationen zu unterstützten Befehlen und ihren Wirkungen, soweit verfügbar, anzeigen.
- Standardisierte, optionale und herstellerspezifische Befehle unterscheiden.
- Für jeden Befehl Lesewirkung, Zustandsänderung, mögliche Datenveränderung und bekannte Grenzen getrennt kennzeichnen.
- „Nicht unterstützt“, „durch Verbindung nicht erreichbar“ und „Wirkung unbekannt“ ausdrücklich unterscheiden.
- Befehlswirkungen nur bei belastbarer Quelle oder konkreter Gerätemeldung als bestätigt darstellen.

## Oberfläche
- Durchsuchbare Expertenmatrix: Befehl, Herkunft, Verfügbarkeit, potenzielle Wirkung, Risikostufe.
- Detailseite mit Erklärung in Alltagssprache und technischer Ansicht.
- Querverweise zu Firmwarelabor, Namespace-Verwaltung und destruktiver Schutzzone.
- Keine Schaltfläche zur freien Befehlsausführung in der normalen Diagnose.

## KI-Verhalten
- Vor einer Empfehlung prüfen, ob die konkrete SSD und der Transport den Befehl tatsächlich unterstützen.
- Potenzielle Datenvernichtung deutlich vor einer bloßen Controllerzustandsänderung hervorheben.
- Keine Befehlswirkung aus dem Namen allein ableiten.
- Bei unbekannter Wirkung keine vermeintlich sichere Standardempfehlung erzeugen.

## Schutzregeln
- Read-only-Diagnose bleibt die Voreinstellung.
- Firmware-, Namespace- und Löschbefehle nur in getrennten ausdrücklich gekennzeichneten Bereichen.
- Keine automatische Befehlsfolge aufgrund einer KI-Vermutung.
- Quellgerät, Zielgerät und Geräteidentität stets klar sichtbar.

## Noch gewünschte Funktionen
Befehlswirkungsmatrix, Sicherheitskennzeichnung, Quellenbelege und Verknüpfung mit Risikobereichen.

## Quelle
NVM Express Spezifikationsübersicht: https://nvmexpress.org/specifications/

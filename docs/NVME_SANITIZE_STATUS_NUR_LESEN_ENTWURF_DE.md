# Sanitize-Status: reine Zustandsanzeige – Produktentwurf

## Ziel
Die Anwendung soll den Status einer möglicherweise bereits laufenden oder früher gestarteten NVMe-Sanitize-Operation erkennen und verständlich erklären, ohne selbst eine Löschung auszulösen.

## Statusinformationen
- Gemeldeter Sanitize-Zustand, soweit verfügbar.
- Laufend, abgeschlossen, fehlgeschlagen oder nicht eindeutig.
- Gemeldeter Fortschritt nur anzeigen, wenn das Gerät einen verlässlichen Wert liefert.
- Letzte bekannte Ereignisse mit Zeitstempel und Quelle.
- Unterschied zwischen Sanitize, Format NVM und normalem Dateisystemformat erläutern.

## Benutzeroberfläche
- Eigene Statuskarte mit deutlich sichtbarer Warnung vor möglichem Datenverlust.
- Anzeige der zugrunde liegenden Logseite oder Systemmeldung.
- Fehlende Log-Unterstützung oder USB-Bridge-Einschränkungen erklären.
- Keine Schaltfläche zum Starten einer Sanitize-Aktion in der normalen Diagnose.
- Bei laufender Operation keine irreführende Zusage machen, dass ein Programmabbruch die Löschung beendet.

## KI-Unterstützung
- Status mit persistenten Ereignissen und Firmware-/Controllerinformationen vergleichen.
- Auswirkungen auf mögliche Datenrettung vorsichtig erläutern.
- Keine Wiederherstellbarkeit bereits gelöschter Daten behaupten.
- Unsichere oder widersprüchliche Statusinformationen offenlegen.

## Schutz
- Nur lesende Zustandsanzeige.
- Keine automatische Wiederholung, Fortsetzung oder Änderung einer Sanitize-Operation.
- Keine Vermischung mit dem getrennten Bereich für datenvernichtende Aktionen.

## Noch gewünschte Funktionen
Sanitize-Statuskarte, Warnungen, Ereignisverknüpfung und klare Abgrenzung zur Datenrettung.

## Quelle
NVM Express Base Specification: https://nvmexpress.org/specification/nvm-express-base-specification/

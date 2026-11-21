# Schutzbereich für datenvernichtende NVMe-Funktionen – Produktentwurf

## Ziel
NVMe Recovery Studio AI muss gefährliche Wartungsfunktionen klar von Datenrettung und normaler Diagnose trennen. Insbesondere Format NVM, Secure Erase, Sanitize und Namespace-Löschung dürfen nicht als harmlose Reparaturmaßnahmen erscheinen.

## Funktionsübersicht
- Eigener Bereich „Datenvernichtende Aktionen“ mit deutlicher Warnkennzeichnung.
- Übersicht der vom konkreten Controller gemeldeten Fähigkeiten, soweit auslesbar.
- Unterschiede zwischen Formatieren, Löschen von Schlüsseln, Blocklöschung und Namespace-Änderung erklären.
- Kennzeichnen, wenn eine Funktion über USB-Bridge nicht zugänglich oder die Unterstützung unbekannt ist.
- Deutlich anzeigen, dass Sanitize nach dem Start weiterlaufen kann und nicht als gewöhnlich abbrechbare Aufgabe behandelt werden darf.

## Schutz der Datenrettung
- Bei einer aktiven Rettungsfallakte keine datenvernichtende Aktion als Standardvorschlag anzeigen.
- Zuvor vorhandene Abbild- und Rettungsergebnisse sichtbar machen.
- Quellgerät und potenziell betroffene Bereiche eindeutig benennen.
- Niemals ein nicht identifiziertes Gerät für solche Aktionen vorauswählen.
- Kein automatischer Aufruf nach Programmstart, Verbindungswiederkehr oder Firmwarefehler.

## Verständliche Entscheidung
- Vor einer potenziellen Datenvernichtung den Unterschied zwischen „SSD wieder nutzbar machen“ und „Daten retten“ erklären.
- Folgen, Unumkehrbarkeit und bekannte Einschränkungen in klarer Sprache darstellen.
- Explizite, getrennte Benutzerfreigabe vorsehen; bloßes Öffnen der Seite gilt nicht als Zustimmung.
- Wenn Geräteidentität oder Umfang unklar bleiben, keine Freigabe anbieten.
- Ergebnisstatus von einer tatsächlichen Wiederherstellung der Nutzerdaten klar trennen.

## KI-Verhalten
- Datenvernichtende Befehle niemals als universelle SSD-Reparatur empfehlen.
- Alternativen zur Datensicherung und professionellen Datenrettung nennen.
- Unterstützungsinformationen und Herstellerbelege sichtbar machen.
- Unsicherheit über Controllerverhalten ausdrücklich anzeigen.

## Noch gewünschte Funktionen
Abgetrennter Sicherheitsbereich, Funktionsvergleich, klare Warnungen und nachvollziehbare Entscheidungen.

## Technische Referenz
NVM Express Base Specification, Revision 2.3, Abschnitt Sanitize und Annex A:
https://nvmexpress.org/wp-content/uploads/NVM-Express-Base-Specification-Revision-2.3-2025.08.01-Ratified.pdf

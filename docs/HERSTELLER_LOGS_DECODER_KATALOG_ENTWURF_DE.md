# Herstellerspezifische NVMe-Logs und Decoder-Katalog – Produktentwurf

## Ziel
Die Anwendung soll zusätzliche herstellerspezifische Diagnoseinformationen nutzen können, ohne unbekannte Bytes oder undokumentierte Felder als gesicherte Erkenntnisse auszugeben.

## Katalog
- Hersteller, Controllerfamilie, Firmwarebereich und Log-Kennung als getrennte Merkmale.
- Dokumentierte Datenfelder mit Einheit, Bedeutung, Wertebereich und Quelle.
- Decoder-Version und unterstützte Gerätevarianten sichtbar machen.
- Hinweise auf erforderliche direkte PCIe-Verbindung oder USB-Bridge-Einschränkungen.
- Fehlende Decoder und unbekannte Logversionen ausdrücklich markieren.

## Oberfläche
- Hersteller-Log-Explorer mit Rohdatenansicht und übersetzten Feldern.
- „Verlässlich dokumentiert“, „teilweise bekannt“ und „nicht interpretierbar“ als Erklärstatus.
- Vergleich zweier Logstände mit markierten Änderungen.
- Verknüpfung zu Controllerwissen, Firmwarebibliothek und Fallakte.

## KI-Verhalten
- Keine Zuordnung allein anhand eines ähnlich klingenden Modellnamens.
- Firmwareversion und mögliche Änderungen am Logformat berücksichtigen.
- Plausible Vermutungen von nachgewiesenen Feldbedeutungen trennen.
- Keine herstellerspezifischen Schreibbefehle als Bestandteil einer Logdiagnose auslösen.
- Nicht verfügbare Logs nicht als Hinweis auf einen Defekt werten.

## Wissenspflege
- Quellen, Herkunft und Gültigkeitsbereich je Decoder dokumentieren.
- Lokale Aktualisierung des Decoder-Katalogs ermöglichen.
- Widersprüchliche Quellen anzeigen statt eine davon stillschweigend zu bevorzugen.
- Offline nutzbare Dokumentation und verständliche deutsche Erläuterungen bereitstellen.

## Noch gewünschte Funktionen
Decoder-Katalog, Log-Explorer, Formatversionen und Quellenvertrauen.

## Quelle
NVM Express: Features for Error Reporting, SMART, Log Pages, Failures and Management Capabilities:
https://nvmexpress.org/resource/features-for-error-reporting-smart-log-pages-failures-and-management-capabilities-in-nvme-architectures/

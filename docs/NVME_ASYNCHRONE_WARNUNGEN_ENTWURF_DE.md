# Asynchrone NVMe-Ereignisse und Live-Warnungen – Produktentwurf

## Ziel
NVMe Recovery Studio AI soll spontan gemeldete Controllerereignisse und deren Zusammenhang mit laufenden Diagnose- oder Rettungssitzungen verständlich darstellen, sofern die Verbindung dies ermöglicht.

## Ereignisübersicht
- Kritische Gesundheitswarnungen und relevante Fehlerereignisse.
- Meldungen über geänderte Log-Informationen oder Zustände.
- Controller-Reset, Verbindungsverlust und Wiedererkennung als separate Hostereignisse.
- Zeitpunkt, Geräteidentität und tatsächliche Quelle je Ereignis.
- Ereignisse, die nur durch spätere Logabfragen bekannt wurden, nicht als live empfangen ausgeben.

## Oberfläche
- Kompakte Warnleiste im Live-Monitor.
- Ereignischronik mit Filtern nach Dringlichkeit, Quelle und Sitzung.
- Details: Was wurde gemeldet, was ist unbekannt, welche Daten liegen vor?
- Verknüpfung zu SMART, Telemetrie, Rechneranalyse und Fallakte.
- Benachrichtigungen dürfen die Bedienung nicht durch dauernde Wiederholungen blockieren.

## KI-Unterstützung
- Neue Warnungen im Kontext der bisherigen SSD- und Rechnerdaten erläutern.
- Bei ernsthaften Risiken Datenrettung und Schonung der SSD hervorheben.
- Nicht jede Warnung automatisch als bevorstehenden Totalausfall bewerten.
- Alternative Ursachen wie Bridge- oder Hostprobleme sichtbar lassen.
- KI-Hypothesen von Controller- und Betriebssystemmeldungen trennen.

## Schutz
- Keine automatische Firmware- oder Sanitize-Aktion als Reaktion auf Warnungen.
- Bei unklarer Geräteidentität keine Aktionen auf ein vermeintlich passendes Laufwerk übertragen.
- Fehlende Event-Unterstützung als Einschränkung anzeigen.

## Noch gewünschte Funktionen
Live-Ereignisleiste, Warnprioritäten, nachvollziehbare Quellen und Fallaktenverknüpfung.

## Quelle
NVM Express: Error Reporting, SMART, Log Pages and Asynchronous Events:
https://nvmexpress.org/resource/features-for-error-reporting-smart-log-pages-failures-and-management-capabilities-in-nvme-architectures/

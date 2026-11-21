# Geräteerkennung und Verbindungen – Produktentwurf

## Ziel
Die Anwendung soll die angeschlossene NVMe-SSD zuverlässig identifizieren und verständlich anzeigen, welche Informationen und Funktionen über die jeweilige Verbindung verfügbar sind.

## Windows 11 – externe USB-NVMe-SSDs
- Übersicht ausschließlich externer USB-NVMe-Laufwerke.
- Anzeige von SSD-Hersteller, Modell, Kapazität, Firmwarestand, Seriennummer und Verbindungsgeschwindigkeit, soweit ermittelbar.
- USB-Gehäuse beziehungsweise Bridge-Chip getrennt vom eigentlichen NVMe-Controller darstellen.
- Erkennbare Bridge-Familien wie JMicron JMS583, Realtek RTL9210/B, ASMedia ASM2362/2364 und VIA VL716 in die Wissensdatenbank aufnehmen.
- Erklären, wenn Diagnose- oder Recovery-Funktionen wegen einer Bridge nicht verfügbar sind.
- Alternative Verbindungshinweise anbieten, wenn ein direkter PCIe-Zugriff nötig ist.
- Interne Laufwerke grundsätzlich nicht als Recovery-Ziele anzeigen.

## Linux – direkte PCIe-Verbindung
- Übersicht der intern angeschlossenen NVMe-Controller und ihrer Namespaces.
- Anzeige von PCIe-Verbindung, Controllerkennung, Firmwarestand und Gesundheitsdaten, soweit verfügbar.
- Unterschiede zwischen physischem Controller, Namespace und Partition verständlich darstellen.
- Mehrere NVMe-Geräte gleichzeitig in der Übersicht anzeigen.
- Bootmedium und nicht ausgewählte Datenträger deutlich kennzeichnen und schützen.

## Gerätedetailseite
- Gerätebild oder neutrales SSD-Symbol.
- Hersteller, Modell, Seriennummer, Kapazität und Firmware.
- Verbindung: USB-Bridge oder PCIe.
- Controller- und NAND-Informationen, soweit bekannt.
- Gesundheitszustand und bekannte Fehlermeldungen.
- Verfügbare Bereiche: Diagnose, Datenrettung, Firmware, Recovery, Fallakten.
- Kennzeichnung unbekannter Angaben statt erfundener Werte.

## Wechsel und Wiedererkennung
- Anschließen und Entfernen von Geräten in der Oberfläche anzeigen.
- Gerät nach Verbindungswechsel wiedererkennen, soweit eindeutige Identifikation möglich ist.
- Frühere Fallakten mit dem passenden Gerät verknüpfen.
- Bei widersprüchlicher Identität warnen.

## Noch offen
Vollständige Geräteansichten, Bridge-Wissensdatenbank, Verbindungsstatus und gerätebezogene Navigation.

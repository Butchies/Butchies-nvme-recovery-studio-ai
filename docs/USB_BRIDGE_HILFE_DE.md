# USB-Bridge-Kompatibilität und Hilfe

## Grundsatz
Ein USB-NVMe-Gehäuse kann Diagnose- oder Firmwarebefehle blockieren. Die Bridge-Chipfamilie allein beweist keine Recovery-Kompatibilität. Adaptermodell, Bridge-Firmware und konkrete Befehle müssen geprüft werden.

## Zu untersuchende Chipfamilien (keine pauschale Recovery-Freigabe)
- JMicron JMS583
- Realtek RTL9210 / RTL9210B
- ASMedia ASM2362 / ASM2364
- VIA Labs VL716

## Anzeige bei inkompatiblem Adapter
- USB Vendor/Product-ID, Bridge-Chip, soweit verifizierbar, Firmwarestand.
- Welche Funktionen tatsächlich funktionieren: SMART, NVMe Identify, Error Log, Firmware Download, Firmware Commit, Vendor Recovery.
- Welcher Befehl nicht durchgereicht wird und welche Antwort zurückkam.
- Welche nachweislich kompatiblen Alternativadapter oder direkte PCIe-Anschlüsse verfügbar sind.
- Bei fehlenden Daten: 'Kompatibilität nicht bestätigt', keine erfundene Empfehlung.

## Windows-Regel
Keine Umgehung der USB-only-Beschränkung. Für direkten PCIe-Zugriff ausschließlich die separate Linux Boot Edition verwenden.

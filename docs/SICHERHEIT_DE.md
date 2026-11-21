# Sicherheitskonzept

- Windows: interne Datenträger niemals öffnen, diagnostizieren oder beschreiben; nur nachgewiesene USB-NVMe-Zielgeräte.
- Linux: alle Geräte zunächst lesend; System-/Datenlaufwerke, Recovery-Stick und unklare Ziele besonders schützen.
- Vor Flash: stabile Geräteidentität (Pfad allein genügt nicht), Bus, Controller-/Board-/NAND-Kompatibilität, Firmwareherkunft, Hash und erforderliche Signaturen verifizieren.
- Die KI hat keine direkten privilegierten Rohschreibrechte. Ein separater Executor akzeptiert nur geprüfte Operationen und eine einmalige, zielgebundene Benutzerfreigabe.
- Firmware-Download/Commit, Vendor-Befehle und eigene Firmware sind potenziell datenvernichtend. Risiken anzeigen und sichere Sicherung empfehlen.
- Keine Endlosschleifen beim Flashen, keine automatischen Experimente mit inkompatiblen Images.
- Eigene Firmware nur für dokumentierte Hardware mit bekanntem Programmierweg. Kein Umgehen kryptografischer Signaturprüfungen.
- Codegenerierung und Test in Sandbox; neue Verfahren zunächst als unbestätigt markieren.
- Offline-Firmwarepakete nur mit geklärten Nutzungs-/Weitergaberechten.
- Audit-Log für jede Aktion mit Zeit, Gerät, Befehl, Bestätigung und Ergebnis.

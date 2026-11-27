# Dell Samsung PM981a

Der ISO-Bau extrahiert `Dell-PM981a-G4GWG-15305129.bin` (1.310.720 Byte,
SHA-256 `08156487302ee23413e2a6ceffc0ba1a94d9e881db55b407c0d81d79fa660db9`).
Die Bytes stammen unverändert aus dem Nutzlastbereich des Dell-Programms
`MUP_PM981a_15305129.exe` (SHA-256
`e016ee21712ce6c3f1aaeb44df03b426a5e5c11094dd019e110a105c5c779792`).
Die enthaltene Versionsmarkierung lautet `15305129`. Das ist eine normale
NVMe-Update-Nutzlast, **kein Phoenix/SM981-Notmodus-Loader oder MPTool**.

Linux kann diese Datei erst über die NVMe-Firmware-Download-Befehle verwenden,
wenn Identify und ein `/dev/nvmeX`-Controller antworten. Der beobachtete
PCIe-Zustand `144d:a808` ohne NVMe-Funktion erfüllt das nicht. Die Zuordnung
des Dell-Abbilds zu Board, NAND und SED-Variante ist ohne ansprechendes
Identify nicht vollständig belegbar. Es erfolgt kein automatisches Flashen.

Zusätzlich werden die originalen Windows-Updatepakete für 512 GB und 1 TB
als Referenz mit Prüfsumme geladen:

- [YF6KT · 15305629 (SED)](https://downloads.dell.com/FOLDER08616140M/2/Samsung-PM981a-512GB-and-1TB-SED-Solid-State-Drive_YF6KT_WIN64_1530.5629_A00_01.EXE)
- [G4GWG · 15305129](https://downloads.dell.com/FOLDER08616185M/2/Samsung-PM981a-512GB-and-1TB-Solid-State-Drive-Firmware_G4GWG_WIN64_1530.5129_A00_01.EXE)

Die EXE-Dateien laufen nicht auf dem Linux-Stick. Für das Linux-Update ist
die `.bin` vorgesehen; bei SED-Geräten darf das G4GWG-Abbild nicht anhand
einer bloßen PCI-ID als passend angenommen werden.

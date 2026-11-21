# Linux-Boot-Edition (USB-Stick)

Startet von einem USB-Stick (UEFI und BIOS) direkt in NVMe Recovery Studio AI im Vollbild – ohne Desktop.

**Was die Linux-Edition zusätzlich kann**
- Direkter PCIe/NVMe-Zugriff auf interne M.2-SSDs (keine USB-Bridge dazwischen).
- Erkennung von Controllern im Notmodus (PCIe-Gerät eines SSD-Controllerherstellers ohne funktionsfähiges NVMe).
- Firmware-Manager: Standard-NVMe-Firmware-Update mit Prüfungen, Bestätigungscode und anschließender Kontrolle.
- Rettungsabbild direkt von der internen SSD; Fortsetzen einer unter Windows begonnenen Rettung.
- Boot-Stick und Systemlaufwerke werden automatisch erkannt und gesperrt.
- WLAN/LAN über NetworkManager; smartmontools, nvme-cli und GNU ddrescue sind enthalten.
- Dauerhafter Speicher auf dem Stick (Partition `persistence`): KI-Wissen, Fallakten, Lesekarten und registrierte Werkzeuge bleiben erhalten.

**Abbild bauen** (Debian 13, Root, Internet):
```
sudo apt install live-build rsync
sudo ./linux-boot/build.sh        # Fortschritt in Prozent; Ergebnis: linux-boot/out/*.iso
```

**Auf Stick schreiben**
- Windows: NVMe Recovery Studio AI → „Linux-Boot-Stick erstellen“ (schreibt, prüft per SHA-256; der dauerhafte Speicher wird beim ersten Start angelegt).
- Linux: `sudo ./linux-boot/make-stick.sh linux-boot/out/nvme-recovery-studio-ai-linux.iso /dev/sdX`

**Starten**: Rechner vom Stick booten (Boot-Menü, meist F8/F11/F12). Secure Boot: Debian-Bootloader ist signiert; bei Problemen Secure Boot vorübergehend deaktivieren.

**Aufbau**
| Datei | Zweck |
|---|---|
| `build.sh` | live-build-Konfiguration und Bau |
| `config/package-lists/recovery.list.chroot` | Pakete (Kiosk, Qt 6, WLAN, Werkzeuge) |
| `config/hooks/live/0100-build-app.hook.chroot` | kompiliert die Anwendung im Abbild |
| `config/includes.chroot/etc/systemd/system/nvme-recovery-kiosk.service` | startet die Anwendung im Vollbild (cage) |
| `config/includes.chroot/usr/local/sbin/nvme-recovery-persistence-setup` | legt beim ersten Start den dauerhaften Speicher an |
| `make-stick.sh` | Stick unter Linux schreiben |

Status: Konfiguration vorhanden, Abbild noch nicht gebaut und nicht auf Hardware getestet.

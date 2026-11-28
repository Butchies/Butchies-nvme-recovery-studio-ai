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
sudo apt install live-build rsync p7zip-full
sudo ./linux-boot/build.sh        # Fortschritt in Prozent; Ergebnis: linux-boot/out/*.iso
```

**Auf Stick schreiben**
- Windows: NVMe Recovery Studio AI → „Linux-Boot-Stick erstellen“ (schreibt, prüft per SHA-256; der dauerhafte Speicher wird beim ersten Start angelegt).
- Linux: `sudo ./linux-boot/make-stick.sh linux-boot/out/nvme-recovery-studio-ai-linux.iso /dev/sdX`

**Starten**: Rechner vom Stick booten (Boot-Menü, meist F8/F11/F12). Secure Boot: Debian-Bootloader ist signiert; bei Problemen Secure Boot vorübergehend deaktivieren.

**Bootreport für die Fehlersuche:** Ab 8 Sekunden nach dem Boot und danach alle
2 Minuten werden PCI/USB-Geräte, Treiberbindungen, fehlende Kernel-Firmware,
NVMe-Erkennung, WLAN/LAN, Startwarnungen und Kiosk-Logs gesammelt. Die Datei
liegt auf der Windows-lesbaren Partition `NVMEDATEN` unter
`Bootreports/Bootreport-<Boot-ID>.txt`. Ein fehlgeschlagenes Anlegen dieser
Partition wird im Windows-Stick-Ersteller als Fehler angezeigt. Der Linux-Stick-
Ersteller legt `NVMEDATEN` auch ohne zusätzlichen Datenordner an. Wird die
Partition nicht eingebunden, liegt der Bericht im gestarteten System unter
`/root/Butchies-Recovery/Bootreports/`; mit einem separaten USB-Stick mit
exFAT/FAT32 und dem Label `BOOTREPORT` kann `sudo butchies-hardware capture`
ihn dorthin kopieren. Ein bereits geschriebener alter Stick erhält diese
Änderungen erst durch erneutes Schreiben des neuen ISO (dabei werden seine
Daten gelöscht).

Der Windows-Installer enthält dasselbe geprüfte Linux-ISO samt Linux-Werkzeugen
und Firmwarearchiv im Unterordner `boot`. „Neueste laden“ prüft zuerst dieses
lokale ISO per SHA-256; bei gültiger Prüfsumme ist kein Download nötig.

**Aufbau**
| Datei | Zweck |
|---|---|
| `build.sh` | live-build-Konfiguration und Bau |
| `config/package-lists/recovery.list.chroot` | Pakete (Kiosk, Qt 6, WLAN, Werkzeuge) |
| `config/hooks/live/0100-build-app.hook.chroot` | kompiliert die Anwendung im Abbild |
| `config/includes.chroot/etc/systemd/system/nvme-recovery-kiosk.service` | startet die Anwendung im Vollbild (cage) |
| `config/includes.chroot/usr/local/sbin/nvme-recovery-persistence-setup` | legt beim ersten Start den dauerhaften Speicher an |
| `make-stick.sh` | Stick unter Linux schreiben |

Das Dell-PM981a-Abbild `Firmware/PM981a/Dell-PM981a-G4GWG-15305129.bin`
liegt als unveränderte Nutzlast mit Manifest und SHA-256-Prüfung im ISO und
wird beim Start ins Firmwarearchiv übernommen. Ein Windows-EXE ist unter
Linux keine ausführbare Firmwaremaßnahme. Die Anwendung verlangt NVMe Identify,
Modellprüfung, freie SSD und eine ausdrückliche Freigabe mit Seriennummerncode.
Bei `144d:a808` ohne NVMe-Geräteknoten ist dieses Update nicht möglich;
ein herstellerspezifischer Notmodus-Loader ist derzeit nicht verfügbar.

Status: Konfiguration vorhanden, Abbild noch nicht gebaut und nicht auf Hardware getestet.

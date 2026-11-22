#!/usr/bin/env bash
# Schreibt das Boot-Abbild unter Linux auf einen USB-Stick und legt direkt den
# dauerhaften Speicher an.   sudo ./linux-boot/make-stick.sh out/….iso /dev/sdX [Datenordner]
# Datenordner (optional): Programmdaten mit Firmware-Archiv, Werkzeugen und KI-Wissen,
# z. B. ~/.local/share/nvme-recovery-studio – landet in Partition NVMEDATEN (offline nutzbar).
set -euo pipefail
ISO="${1:?Abbild angeben}"; DEV="${2:?Stick angeben, z. B. /dev/sdb}"; DATA="${3:-}"
[ "$(id -u)" -eq 0 ] || { echo "Bitte mit sudo ausführen."; exit 1; }
[ "$(lsblk -dno TRAN "$DEV")" = "usb" ] || { echo "$DEV ist kein USB-Datenträger – abgebrochen."; exit 1; }
if lsblk -no MOUNTPOINT "$DEV" | grep -q .; then echo "$DEV hat eingebundene Partitionen – bitte aushängen."; exit 1; fi
lsblk -o NAME,SIZE,MODEL,TRAN "$DEV"
read -r -p "ALLE Daten auf $DEV werden gelöscht. Zur Bestätigung STICK eingeben: " ok
[ "$ok" = "STICK" ] || { echo "Abgebrochen."; exit 1; }
dd if="$ISO" of="$DEV" bs=4M conv=fsync oflag=direct status=progress
sync; partprobe "$DEV"; sleep 2
END=$(parted -sm "$DEV" unit MiB print | awk -F: 'NR>2{gsub("MiB","",$3); if($3>m)m=$3} END{print int(m)+1}')
if [ -n "$DATA" ] && [ -d "$DATA" ]; then
  MB=$(( $(du -sm "$DATA" | cut -f1) + 2048 )); [ "$MB" -ge 4096 ] || MB=4096
  parted -s "$DEV" mkpart primary "${END}MiB" "$((END+MB))MiB"
  partprobe "$DEV"; sleep 2
  DPART="$(lsblk -lnpo NAME "$DEV" | tail -n1)"
  mkfs.exfat -n NVMEDATEN "$DPART"
  mkdir -p /tmp/nvmedaten && mount "$DPART" /tmp/nvmedaten
  echo "Kopiere Offline-Paket (Firmware-Archiv, Werkzeuge, KI-Wissen) …"
  rsync -r --info=progress2 --exclude boot/ "$DATA"/ /tmp/nvmedaten/nvme-recovery-studio/
  umount /tmp/nvmedaten
  END=$((END+MB))
fi
parted -s "$DEV" mkpart primary ext4 "${END}MiB" 100%
partprobe "$DEV"; sleep 2
PART="$(lsblk -lnpo NAME "$DEV" | tail -n1)"
mkfs.ext4 -F -L persistence "$PART"
mkdir -p /tmp/persistence && mount "$PART" /tmp/persistence
echo "/root union" > /tmp/persistence/persistence.conf
umount /tmp/persistence
echo "Fertig: $DEV ist ein Linux-Boot-Stick mit dauerhaftem Speicher ($PART)."

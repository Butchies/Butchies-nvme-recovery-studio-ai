#!/usr/bin/env bash
# Baut die Linux-Boot-Edition von NVMe Recovery Studio AI als UEFI-bootfähiges
# Hybrid-ISO (auf USB-Stick schreibbar). Benötigt einen Debian-13-Rechner
# (oder Container) mit Root-Rechten und Internetzugang:
#   sudo apt install live-build
#   sudo ./linux-boot/build.sh
# Ausgabe: linux-boot/out/nvme-recovery-studio-ai-linux.iso
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
REPO="$(cd "$HERE/.." && pwd)"
WORK="$HERE/work"
OUT="$HERE/out"

step=0; steps=7
progress() {           # Fortschritt in Prozent mit Ladebalken
  step=$((step+1)); local p=$((step*100/steps)); local bars=$((p/5))
  printf '[%-20s] %3d %%  %s\n' "$(printf '%*s' "$bars" '' | tr ' ' '#')" "$p" "$1"
}

[ "$(id -u)" -eq 0 ] || { echo "Bitte mit sudo ausführen."; exit 1; }
command -v lb >/dev/null || { echo "live-build fehlt: sudo apt install live-build"; exit 1; }

progress "Arbeitsverzeichnis vorbereiten"
rm -rf "$WORK"; mkdir -p "$WORK" "$OUT"
cd "$WORK"

progress "live-build konfigurieren (Debian 13, UEFI + BIOS, Hybrid-ISO)"
lb config \
  --distribution trixie \
  --archive-areas "main contrib non-free non-free-firmware" \
  --binary-images iso-hybrid \
  --bootloaders "grub-efi,syslinux" \
  --debian-installer none \
  --memtest none \
  --iso-application "NVMe Recovery Studio AI" \
  --iso-volume "NVMERECOVERY" \
  --bootappend-live "boot=live components persistence persistence-label=persistence quiet splash nvme_core.default_ps_max_latency_us=0 pcie_aspm=off"

progress "Paketlisten, Dienste und Programmquellen übernehmen"
cp -a "$HERE/config/." "$WORK/config/"
mkdir -p "$WORK/config/includes.chroot/opt/src"
rsync -a --exclude '.git' --exclude 'linux-boot/work' --exclude 'linux-boot/out' \
  "$REPO/native" "$REPO/data" "$WORK/config/includes.chroot/opt/src/"
echo "${APP_VERSION:-0.0.0}" > "$WORK/config/includes.chroot/opt/src/VERSION"

progress "System bauen (lädt Pakete, kompiliert die Anwendung – dauert länger)"
set +e
lb build > >(tee "$OUT/build.log") 2>&1
build_status=$?
set -e
if [ "$build_status" -ne 0 ]; then
  echo "Linux-ISO-Build fehlgeschlagen (Exit-Code $build_status). Protokoll: $OUT/build.log" >&2
  exit "$build_status"
fi
printf "Build erfolgreich. Protokoll: %s\n" "$OUT/build.log"

progress "Ergebnis prüfen"
ISO="$(ls -1 "$WORK"/*.iso 2>/dev/null | head -n1 || true)"
[ -n "$ISO" ] || { echo "Build fehlgeschlagen – siehe $OUT/build.log"; exit 1; }

progress "Abbild ablegen"
mv "$ISO" "$OUT/nvme-recovery-studio-ai-linux.iso"
sha256sum "$OUT/nvme-recovery-studio-ai-linux.iso" > "$OUT/nvme-recovery-studio-ai-linux.iso.sha256"

progress "Fertig"
echo "Abbild: $OUT/nvme-recovery-studio-ai-linux.iso"
echo "Auf Stick schreiben: Windows-Edition → „Linux-Boot-Stick erstellen“ oder linux-boot/make-stick.sh"

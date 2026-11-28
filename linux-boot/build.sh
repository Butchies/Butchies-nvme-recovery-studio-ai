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
firmware_source="$REPO/Firmware/PM981a"
firmware_target="$WORK/config/includes.binary/Firmware/PM981a"
mkdir -p "$firmware_target"
cp "$firmware_source/README.md" "$firmware_source/downloads.txt" \
  "$firmware_source/index.json" "$firmware_target/"
command -v 7z >/dev/null || { echo '7z fehlt: sudo apt install p7zip-full' >&2; exit 1; }
while read -r checksum url; do
    package="$firmware_target/${url##*/}"
    curl --fail --location --retry 3 --max-time 120 --user-agent 'Mozilla/5.0' \
        --referer 'https://www.dell.com/' --output "$package" "$url"
    printf '%s  %s\n' "$checksum" "$package" | sha256sum --check
done < "$firmware_source/downloads.txt"
tmp_firmware="$(mktemp -d)"
trap 'rm -rf "$tmp_firmware"' EXIT
g4gwg="$(find "$firmware_target" -maxdepth 1 -name '*G4GWG*EXE' -print -quit)"
[ -n "$g4gwg" ] || { echo 'Dell G4GWG-Paket fehlt' >&2; exit 1; }
7z x -y -o"$tmp_firmware" "$g4gwg" >/dev/null
inner="$(find "$tmp_firmware" -type f -iname 'MUP_PM981a_15305129.exe' -print -quit)"
[ -n "$inner" ] || { echo 'MUP_PM981a_15305129.exe im Dell-Paket nicht gefunden' >&2; exit 1; }
printf '%s  %s\n' 'e016ee21712ce6c3f1aaeb44df03b426a5e5c11094dd019e110a105c5c779792' "$inner" | sha256sum --check
python3 - "$inner" "$firmware_target/Dell-PM981a-G4GWG-15305129.bin" <<'PY'
import pathlib, sys
source=pathlib.Path(sys.argv[1]).read_bytes()
image=source[0x8d8d0:0x1cd8d0]
assert len(image)==0x140000 and b'FW_IMAGE_META_SIGNATURE_________15305129' in image
pathlib.Path(sys.argv[2]).write_bytes(image)
PY
(cd "$firmware_target" && echo '08156487302ee23413e2a6ceffc0ba1a94d9e881db55b407c0d81d79fa660db9  Dell-PM981a-G4GWG-15305129.bin' | sha256sum --check)
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

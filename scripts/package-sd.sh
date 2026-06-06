#!/usr/bin/env bash
#
# package-sd.sh — собирает полную SD-раскладку hekate (модуль экосистемы Ряженка).
#
# Складывает в <dest>:
#   payload.bin                         — собранный payload (для autoboot/RCM)
#   bootloader/update.bin               — тот же payload (автообновление модчипа)
#   bootloader/sys/nyx.bin              — собранный Nyx GUI
#   bootloader/sys/libsys_lp0.bso       — собранный модуль LP0
#   bootloader/sys/libsys_minerva.bso   — собранный модуль Minerva
#   bootloader/sys/{emummc.kipm,res.pak,thk.bin,l4t/*} — prebuilt (res/sd)
#   bootloader/res/{icon_switch,icon_payload}.bmp      — иконки Nyx (res/sd)
#   bootloader/{hekate_ipl_template,patches_template}.ini — шаблоны
#
# Использование: scripts/package-sd.sh <dest-dir>
#
set -euo pipefail

DEST="${1:?usage: package-sd.sh <dest-dir>}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"

rm -rf "$DEST"
mkdir -p "$DEST/bootloader"

# 1. Prebuilt SD skeleton (res.pak, emummc.kipm, thk.bin, l4t/, default icons,
#    and a fallback autokeys Lockpick build).
cp -r "$ROOT/res/sd/bootloader/." "$DEST/bootloader/"

# Prefer a freshly built patched Lockpick (autokeys) if CI produced one.
if [ -f "$ROOT/output/lockpick.bin" ]; then
  cp "$ROOT/output/lockpick.bin" "$DEST/bootloader/sys/lockpick.bin"
fi

# 2. Overlay the freshly built binaries.
mkdir -p "$DEST/bootloader/sys"
cp "$ROOT/output/nyx.bin"            "$DEST/bootloader/sys/nyx.bin"
cp "$ROOT/output/libsys_lp0.bso"     "$DEST/bootloader/sys/libsys_lp0.bso"
cp "$ROOT/output/libsys_minerva.bso" "$DEST/bootloader/sys/libsys_minerva.bso"

# 3. Config templates.
cp "$ROOT/res/hekate_ipl_template.ini" "$DEST/bootloader/"
cp "$ROOT/res/patches_template.ini"    "$DEST/bootloader/"

# 4. Empty user folders (kept for layout clarity).
mkdir -p "$DEST/bootloader/ini" "$DEST/bootloader/payloads"

# 5. Payload at the SD root + update.bin for modchip auto-update.
cp "$ROOT/output/hekate.bin" "$DEST/payload.bin"
cp "$ROOT/output/hekate.bin" "$DEST/bootloader/update.bin"

echo "Packaged SD layout into $DEST"

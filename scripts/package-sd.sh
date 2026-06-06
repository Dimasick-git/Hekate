#!/usr/bin/env bash
#
# package-sd.sh — собирает полную SD-раскладку hekate (модуль экосистемы Ряженка).
#
# Складывает в <dest>:
#   payload.bin / bootloader/update.bin — собранный payload (autoboot/RCM, автообновление)
#   bootloader/sys/{nyx.bin,libsys_lp0.bso,libsys_minerva.bso} — собранные бинарники
#   bootloader/sys/lockpick.bin         — патченный Lockpick (autokeys); CI кладёт 1.9.20,
#                                         иначе берётся fallback из res/sd
#   bootloader/sys/{emummc.kipm,res.pak,thk.bin,l4t/*} — prebuilt из res/sd
#   bootloader/{hekate_ipl.ini,nyx.ini}, bootloader/ini/*, bootloader/res/*,
#   bootloader/payloads/*               — готовая конфигурация и ресурсы из res/sd
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

# 3. Payload at the SD root + update.bin for modchip auto-update.
cp "$ROOT/output/hekate.bin" "$DEST/payload.bin"
cp "$ROOT/output/hekate.bin" "$DEST/bootloader/update.bin"

echo "Packaged SD layout into $DEST"

#!/usr/bin/env bash
#
# package-sd.sh — собирает полную SD-раскладку hekate (модуль экосистемы Ряженка).
#
# Складывает в <dest>:
#   payload.bin / bootloader/update.bin — собранный payload (autoboot/RCM, автообновление)
#   bootloader/sys/{nyx.bin,libsys_lp0.bso,libsys_minerva.bso} — собранные бинарники
#   bootloader/sys/lockpick.bin         — патченный Lockpick 2.0.1 (autokeys); CI кладёт
#                                         свежую проверенную сборку; без неё упаковка прерывается
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
#    and resources).
cp -r "$ROOT/res/sd/bootloader/." "$DEST/bootloader/"

# Require the freshly built patched Lockpick for both automatic and manual use.
test -s "$ROOT/output/lockpick.bin" || { echo "Build patched Lockpick before packaging." >&2; exit 1; }
cp "$ROOT/output/lockpick.bin" "$DEST/bootloader/sys/lockpick.bin"
mkdir -p "$DEST/bootloader/payloads"
cp "$ROOT/output/lockpick.bin" "$DEST/bootloader/payloads/Lockpick_RCM.bin"

# 2. Overlay the freshly built binaries.
mkdir -p "$DEST/bootloader/sys"
cp "$ROOT/output/nyx.bin"            "$DEST/bootloader/sys/nyx.bin"
cp "$ROOT/output/libsys_lp0.bso"     "$DEST/bootloader/sys/libsys_lp0.bso"
cp "$ROOT/output/libsys_minerva.bso" "$DEST/bootloader/sys/libsys_minerva.bso"

# 3. Payload at the SD root + update.bin for modchip auto-update.
cp "$ROOT/output/hekate.bin" "$DEST/payload.bin"
cp "$ROOT/output/hekate.bin" "$DEST/bootloader/update.bin"

echo "Packaged SD layout into $DEST"

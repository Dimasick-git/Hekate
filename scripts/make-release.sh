#!/usr/bin/env bash
#
# make-release.sh — крафтит GitHub-релиз hekate (модуль экосистемы Ряженка) под текущую версию.
#
# Версия берётся из Versions.inc (BLVERSION_MAJOR.MINOR.HOTFX). Тег релиза равен
# этой версии. Если релиза с таким тегом ещё нет — он создаётся. Если уже есть —
# файлы (ассеты) в нём просто перезаписываются (--clobber), сам релиз/тег не
# пересоздаётся.
#
# Требуется gh CLI с аутентификацией (в CI — переменная окружения GH_TOKEN).
#
# Использование:
#   scripts/make-release.sh <папка-с-ассетами> [target-commit-ish]
#
# Переменные окружения (необязательные):
#   TAG_PREFIX   префикс тега (по умолчанию пусто, т.е. тег = "6.5.2")
#
set -euo pipefail

ASSET_DIR="${1:?Укажи папку с файлами релиза: scripts/make-release.sh <dir> [target]}"
TARGET="${2:-}"
TAG_PREFIX="${TAG_PREFIX:-}"

log() { printf '\033[1;35m[release]\033[0m %s\n' "$*"; }

# 1. Читаем версию из Versions.inc.
ver_field() { grep -E "^$1" Versions.inc | grep -oE '[0-9]+' | head -n1; }
MJ="$(ver_field BLVERSION_MAJOR)"
MN="$(ver_field BLVERSION_MINOR)"
HF="$(ver_field BLVERSION_HOTFX)"
VERSION="${MJ}.${MN}.${HF}"
TAG="${TAG_PREFIX}${VERSION}"
log "Версия: ${VERSION}  →  тег: ${TAG}"

# 2. Собираем список ассетов.
mapfile -t ASSETS < <(find "$ASSET_DIR" -maxdepth 1 -type f \( -name '*.zip' -o -name '*.bin' -o -name '*.7z' \) | sort)
if [ "${#ASSETS[@]}" -eq 0 ]; then
  echo "::error::В $ASSET_DIR нет файлов релиза (*.zip / *.bin)." >&2
  exit 1
fi
log "Ассеты:"; printf '  - %s\n' "${ASSETS[@]}"

TITLE="hekate ${VERSION}"
NOTES=$(cat <<EOF
**hekate ${VERSION}** — модуль экосистемы **Ряженка** (форк hekate/Nyx).

Релиз собран автоматически из ветки/коммита \`${TARGET:-$VERSION}\`.

> При повторной публикации этой же версии файлы релиза просто перезаписываются.
EOF
)

# 3. Создаём релиз или перезаписываем ассеты существующего.
if gh release view "$TAG" >/dev/null 2>&1; then
  log "Релиз $TAG уже существует — перезаписываю файлы (--clobber)."
  gh release upload "$TAG" "${ASSETS[@]}" --clobber
  gh release edit "$TAG" --title "$TITLE" --notes "$NOTES" >/dev/null
  log "Готово: ассеты релиза $TAG обновлены."
else
  log "Релиза $TAG нет — создаю новый."
  CREATE_ARGS=( "$TAG" "${ASSETS[@]}" --title "$TITLE" --notes "$NOTES" )
  [ -n "$TARGET" ] && CREATE_ARGS+=( --target "$TARGET" )
  gh release create "${CREATE_ARGS[@]}"
  log "Готово: релиз $TAG создан."
fi

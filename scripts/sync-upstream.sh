#!/usr/bin/env bash
#
# sync-upstream.sh — синхронизация форка Ряженка с оригиналом CTCaer/hekate.
#
# Скрипт подтягивает свежие коммиты из апстрима и сливает их в отдельную ветку
# синхронизации, СОХРАНЯЯ наши файлы (README, CI и т.д.) — за это отвечает
# драйвер слияния "ours" вместе с правилами в .gitattributes.
#
# Ничего не пушит и не открывает PR сам — только готовит ветку слияния и
# сообщает результат. Пуш/PR делает вызывающая сторона (workflow или человек).
#
# Переменные окружения (все необязательные):
#   UPSTREAM_URL     URL апстрима      (по умолчанию https://github.com/CTCaer/hekate.git)
#   UPSTREAM_BRANCH  ветка апстрима    (по умолчанию master)
#   SYNC_BRANCH      ветка слияния     (по умолчанию sync/upstream)
#   BASE_REF         на что опирать sync-ветку (по умолчанию текущий HEAD)
#
# Коды возврата:
#   0 — есть новые изменения, ветка слияния готова к пушу/PR
#   3 — изменений нет, всё актуально
#   2 — остались конфликты в незащищённых файлах, нужно ручное вмешательство
#
set -euo pipefail

UPSTREAM_URL="${UPSTREAM_URL:-https://github.com/CTCaer/hekate.git}"
UPSTREAM_BRANCH="${UPSTREAM_BRANCH:-master}"
SYNC_BRANCH="${SYNC_BRANCH:-sync/upstream}"

log() { printf '\033[1;36m[sync]\033[0m %s\n' "$*"; }

# Записать ключ=значение в $GITHUB_OUTPUT, если мы в GitHub Actions.
emit() {
  if [ -n "${GITHUB_OUTPUT:-}" ]; then
    printf '%s=%s\n' "$1" "$2" >> "$GITHUB_OUTPUT"
  fi
}

# 1. Включаем драйвер слияния "ours" — он всегда оставляет НАШУ версию для
#    файлов, помеченных `merge=ours` в .gitattributes.
git config merge.ours.driver true

# 2. Базовая точка для ветки слияния — то, на чём мы стоим сейчас.
BASE_REF="${BASE_REF:-$(git rev-parse --abbrev-ref HEAD)}"
log "Базовая ветка: $BASE_REF"

# 3. Подключаем и обновляем апстрим.
if git remote get-url upstream >/dev/null 2>&1; then
  git remote set-url upstream "$UPSTREAM_URL"
else
  git remote add upstream "$UPSTREAM_URL"
fi
log "Загрузка $UPSTREAM_URL ($UPSTREAM_BRANCH)…"
git fetch --no-tags upstream "$UPSTREAM_BRANCH"

UPSTREAM_HEAD="$(git rev-parse "upstream/$UPSTREAM_BRANCH")"
log "Апстрим HEAD: $UPSTREAM_HEAD"

# 4. Создаём/пересоздаём ветку слияния поверх нашей базы.
git switch -C "$SYNC_BRANCH" "$BASE_REF"

# 5. Уже всё влито? Тогда выходим без изменений.
if git merge-base --is-ancestor "$UPSTREAM_HEAD" HEAD; then
  log "Изменений нет — форк уже содержит весь апстрим."
  emit has_changes "false"
  emit conflicts "false"
  emit upstream_head "$UPSTREAM_HEAD"
  exit 3
fi

# 6. Сливаем апстрим. Драйвер "ours" сохранит наши защищённые файлы.
log "Слияние upstream/$UPSTREAM_BRANCH в $SYNC_BRANCH…"
if git merge --no-edit --no-ff \
     -m "sync: подтянуть upstream CTCaer/hekate @ ${UPSTREAM_HEAD:0:10}" \
     "upstream/$UPSTREAM_BRANCH"; then
  log "Слияние прошло чисто (наши файлы сохранены)."
  emit has_changes "true"
  emit conflicts "false"
  emit upstream_head "$UPSTREAM_HEAD"
  exit 0
fi

# 7. Остались конфликты в незащищённых файлах — откатываемся и сообщаем.
CONFLICTS="$(git diff --name-only --diff-filter=U || true)"
log "ВНИМАНИЕ: конфликты требуют ручного разрешения:"
printf '%s\n' "$CONFLICTS"
git merge --abort
emit has_changes "true"
emit conflicts "true"
emit upstream_head "$UPSTREAM_HEAD"
exit 2

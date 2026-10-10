#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
bash -n "$root/linux/AppRun"
node --test "$root/tests/"*.test.mjs
executable="$(mktemp "${TMPDIR:-/tmp}/melty-i18n-test.XXXXXX")"
trap 'rm -f -- "$executable"' EXIT
cc -O2 -Wall -Wextra "$root/tests/i18n.test.c" -o "$executable"
"$executable"

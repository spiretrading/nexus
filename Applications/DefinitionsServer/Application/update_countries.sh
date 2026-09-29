#!/usr/bin/env bash
set -euo pipefail
URL="https://datahub.io/core/country-codes/_r/-/data/country-codes.csv"
PARSER="country_code_parser.py"
OUT="countries.yml"
command -v curl >/dev/null || { echo "curl not found"; exit 1; }
[ -f "$PARSER" ] || { echo "Missing $PARSER"; exit 1; }
output=$(mktemp "$OUT.XXXXXX")
trap 'rm -f "$output"' EXIT
trap 'exit 1' HUP INT TERM
curl -fsSL "$URL" | python3 "$PARSER" > "$output"
mv -f "$output" "$OUT"

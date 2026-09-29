#!/bin/bash
set -o errexit
set -o pipefail
APP_NAME="$1"
if [[ -z "$APP_NAME" ]]; then
  echo "Error: Application name required."
  exit 1
fi
DIRECTORY="$(cd -P "$(dirname "${BASH_SOURCE[0]}")" >/dev/null && pwd -P)"
MASTER_REF=""
for ref in master origin/master; do
  if git -C "$DIRECTORY/.." rev-parse --verify --quiet "$ref" >/dev/null; then
    MASTER_REF="$ref"
    break
  fi
done
if [[ -n "$MASTER_REF" ]]; then
  BASE_REF=$(git -C "$DIRECTORY/.." merge-base HEAD "$MASTER_REF")
  BASE=$(git -C "$DIRECTORY/.." rev-list --count --first-parent "$BASE_REF")
  EXTRA=$(git -C "$DIRECTORY/.." \
    rev-list --count --no-merges "$BASE_REF"..HEAD)
  VERSION=$((BASE + EXTRA))
else
  VERSION=$(git -C "$DIRECTORY/.." rev-list --count --first-parent HEAD)
fi
if [[ ! -f "Version.hpp" ]] ||
    ! grep -Fqx "#define ${APP_NAME}_VERSION \"$VERSION\"" "Version.hpp"; then
  echo "#define ${APP_NAME}_VERSION \"$VERSION\"" > "Version.hpp"
fi

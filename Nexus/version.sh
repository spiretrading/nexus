#!/bin/bash
set -o errexit
set -o pipefail
APP_NAME="$1"
if [[ -z "$APP_NAME" ]]; then
  echo "Error: Application name required."
  exit 1
fi
DIRECTORY="$(cd -P "$(dirname "${BASH_SOURCE[0]}")" >/dev/null && pwd -P)"
GIT_DIR="$DIRECTORY/../.git"
MASTER_REF=""
for ref in master origin/master; do
  if git --git-dir="$GIT_DIR" rev-parse --verify --quiet "$ref" >/dev/null; then
    MASTER_REF="$ref"
    break
  fi
done
if [[ -n "$MASTER_REF" ]]; then
  BASE_REF=$(git --git-dir="$GIT_DIR" merge-base HEAD "$MASTER_REF")
  BASE=$(git --git-dir="$GIT_DIR" rev-list --count --first-parent "$BASE_REF")
  EXTRA=$(git --git-dir="$GIT_DIR" \
    rev-list --count --no-merges "$BASE_REF"..HEAD)
  VERSION=$((BASE + EXTRA))
else
  VERSION=$(git --git-dir="$GIT_DIR" rev-list --count --first-parent HEAD)
fi
if [[ ! -f "Version.hpp" ]] || ! grep -q "$VERSION" "Version.hpp"; then
  echo "#define ${APP_NAME}_VERSION \"$VERSION\"" > "Version.hpp"
fi

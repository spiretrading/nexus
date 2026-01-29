#!/bin/bash
set -o errexit
set -o pipefail
APP_NAME="$1"
if [[ -z "$APP_NAME" ]]; then
  echo "Error: Application name required."
  exit 1
fi
DIRECTORY="$(cd -P "$(dirname "${BASH_SOURCE[0]}")" >/dev/null && pwd -P)"
VERSION=$(git --git-dir="$DIRECTORY/../.git" \
  rev-list --count --first-parent HEAD)
if [[ ! -f "Version.hpp" ]] || ! grep -q "$VERSION" "Version.hpp"; then
  echo "#define ${APP_NAME}_VERSION \"$VERSION\"" > "Version.hpp"
fi

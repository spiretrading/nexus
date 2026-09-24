#!/bin/bash
set -o errexit
set -o pipefail
SOURCE="${BASH_SOURCE[0]}"
while [[ -h "$SOURCE" ]]; do
  DIRECTORY="$(cd -P "$(dirname "$SOURCE")" >/dev/null && pwd -P)"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIRECTORY/$SOURCE"
done
DIRECTORY="$(cd -P "$(dirname "$SOURCE")" >/dev/null && pwd -P)"
exec node "$DIRECTORY/../build.js" library build "$@"

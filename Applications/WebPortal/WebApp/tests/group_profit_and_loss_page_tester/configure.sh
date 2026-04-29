#!/bin/bash
set -o errexit
set -o pipefail
SCRIPT_DIR="$(cd -P "$(dirname "${BASH_SOURCE[0]}")" > /dev/null 2>&1 && \
  pwd -P)"
exec "$SCRIPT_DIR/../../application/configure.sh" -D="$SCRIPT_DIR" "$@"

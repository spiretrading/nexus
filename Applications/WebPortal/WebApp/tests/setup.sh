#!/bin/bash
set -o errexit
set -o pipefail

main() {
  resolve_paths
  "$SCRIPT_DIR/../setup.sh"
  setup_library
}

resolve_paths() {
  SCRIPT_DIR="$(cd -P "$(dirname "${BASH_SOURCE[0]}")" > /dev/null 2>&1 && \
    pwd -P)"
  ROOT="$(pwd -P)"
}

setup_library() {
  if [[ ! -d "library" ]]; then
    mkdir library
    pushd library > /dev/null
    "$SCRIPT_DIR/../library/configure.sh" -DD="$ROOT"
    popd > /dev/null
  fi
}

main "$@"

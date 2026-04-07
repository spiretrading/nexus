#!/bin/bash
set -o errexit
set -o pipefail
DIRECTORY=""
ROOT=""

main() {
  resolve_paths
  "$DIRECTORY/../setup.sh" || return 1
  configure_library
}

resolve_paths() {
  local source="${BASH_SOURCE[0]}"
  while [[ -h "$source" ]]; do
    local dir="$(cd -P "$(dirname "$source")" >/dev/null && pwd -P)"
    source="$(readlink "$source")"
    [[ $source != /* ]] && source="$dir/$source"
  done
  DIRECTORY="$(cd -P "$(dirname "$source")" >/dev/null && pwd -P)"
  ROOT="$(pwd -P)"
}

configure_library() {
  if [[ ! -d "library" ]]; then
    mkdir -p library || return 1
    pushd library > /dev/null
    "$DIRECTORY/../library/configure.sh" -DD="$ROOT" ||
      { popd > /dev/null; return 1; }
    popd > /dev/null
  fi
}

main "$@"

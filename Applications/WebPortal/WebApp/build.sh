#!/bin/bash
set -o errexit
set -o pipefail
DIRECTORY=""
ROOT=""

main() {
  resolve_paths
  create_forwarding_scripts
  build_function "$@" "library"
  build_function "$@" "application"
  build_function "$@" "tests"
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

create_forwarding_scripts() {
  if [[ ! -f "build.sh" ]]; then
    ln -s "$DIRECTORY/build.sh" build.sh
  fi
  if [[ ! -f "configure.sh" ]]; then
    ln -s "$DIRECTORY/configure.sh" configure.sh
  fi
}

build_function() {
  local location="${*: -1}"
  if [[ ! -d "$location" ]]; then
    mkdir -p "$location"
  fi
  pushd "$location" > /dev/null
  "$DIRECTORY/$location/build.sh" -DD="$ROOT/library/Dependencies" "${@:1:$#-1}"
  popd > /dev/null
}

main "$@"

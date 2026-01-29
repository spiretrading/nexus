#!/bin/bash
set -o errexit
set -o pipefail
DIRECTORY=""
ROOT=""

main() {
  resolve_paths
  create_forwarding_scripts
  local projects=(
    "library"
    "tests"
    "application"
  )
  for project in "${projects[@]}"; do
    configure_target "$project" "$@"
  done
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

configure_target() {
  local project="$1"
  shift
  if [[ ! -d "$project" ]]; then
    mkdir -p "$project"
  fi
  pushd "$project" > /dev/null
  "$DIRECTORY/$project/configure.sh" -DD="$ROOT/library/Dependencies" "$@"
  popd > /dev/null
}

main "$@"

#!/bin/bash
set -o errexit
set -o pipefail
ROOT=""
DIRECTORY=""
DEPENDENCIES=""

main() {
  resolve_paths
  create_forwarding_scripts
  parse_args "$@"
  setup_dependencies
  create_symlinks
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

parse_args() {
  for i in "$@"; do
    case $i in
      -DD=*)
        DEPENDENCIES="${i#*=}"
        if [[ -z "$DEPENDENCIES" ]]; then
          echo "Error: -DD requires a path argument."
          exit 1
        fi
        ;;
    esac
  done
  if [[ -z "$DEPENDENCIES" ]]; then
    DEPENDENCIES="$ROOT/Dependencies"
  fi
}

setup_dependencies() {
  if [[ ! -d "$DEPENDENCIES" ]]; then
    mkdir -p "$DEPENDENCIES"
  fi
  pushd "$DEPENDENCIES" > /dev/null
  "$DIRECTORY/setup.sh" || { popd > /dev/null; return 1; }
  popd > /dev/null
  if [[ "$DEPENDENCIES" != "$ROOT/Dependencies" ]] && [[ ! -d Dependencies ]]; then
    ln -s "$DEPENDENCIES" Dependencies
  fi
}

create_symlinks() {
  if [[ "$DIRECTORY" != "$ROOT" ]]; then
    if [[ ! -d "source" ]]; then
      ln -s "$DIRECTORY/source" source
    fi
    if [[ ! -f "package.json" ]]; then
      ln -s "$DIRECTORY/package.json" package.json
    fi
    if [[ ! -f "tsconfig.json" ]]; then
      ln -s "$DIRECTORY/tsconfig.json" tsconfig.json
    fi
  fi
}

main "$@"

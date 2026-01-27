#!/bin/bash
set -o errexit
set -o pipefail

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
    local dir
    dir="$(cd -P "$(dirname "$source")" > /dev/null 2>&1 && pwd -P)"
    source="$(readlink "$source")"
    [[ "$source" != /* ]] && source="$dir/$source"
  done
  SCRIPT_DIR="$(cd -P "$(dirname "$source")" > /dev/null 2>&1 && pwd -P)"
  DIRECTORY="$SCRIPT_DIR"
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
  DEPENDENCIES=""
  while [[ "$#" -gt 0 ]]; do
    case "$1" in
      -DD=*)
        DEPENDENCIES="${1#*=}"
        shift
        ;;
      -DD)
        DEPENDENCIES="$2"
        shift 2
        ;;
      *)
        shift
        ;;
    esac
  done
}

setup_dependencies() {
  if [[ -z "$DEPENDENCIES" ]]; then
    DEPENDENCIES="$ROOT/Dependencies"
  fi
  if [[ ! -d "$DEPENDENCIES" ]]; then
    mkdir -p "$DEPENDENCIES"
  fi
  pushd "$DEPENDENCIES" > /dev/null
  "$SCRIPT_DIR/setup.sh"
  popd > /dev/null
}

create_symlinks() {
  if [[ "$DEPENDENCIES" != "$ROOT/Dependencies" ]]; then
    if [[ ! -d "Dependencies" ]]; then
      ln -s "$DEPENDENCIES" Dependencies
    fi
  fi
  if [[ "$DIRECTORY" != "$ROOT" ]]; then
    rm -rf source 2> /dev/null || true
    ln -s "$DIRECTORY/source" source
  fi
}

main "$@"

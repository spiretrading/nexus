#!/bin/bash
set -o errexit
set -o pipefail
ROOT=""
DIRECTORY=""
SCRIPT_DIR=""
DEPENDENCIES=""

main() {
  resolve_paths
  create_forwarding_scripts
  parse_args "$@"
  setup_dependencies || return 1
  create_symlinks || return 1
}

resolve_paths() {
  local source="${BASH_SOURCE[0]}"
  while [[ -h "$source" ]]; do
    local dir="$(cd -P "$(dirname "$source")" >/dev/null && pwd -P)"
    source="$(readlink "$source")"
    [[ $source != /* ]] && source="$dir/$source"
  done
  SCRIPT_DIR="$(cd -P "$(dirname "$source")" >/dev/null && pwd -P)"
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
  while [[ $# -gt 0 ]]; do
    case "$1" in
      -DD=*) DEPENDENCIES="${1#*=}" ;;
      -DD)   DEPENDENCIES="$2"; shift ;;
      -D=*)  DIRECTORY="${1#*=}" ;;
      -D)    DIRECTORY="$2"; shift ;;
    esac
    shift
  done
  if [[ -z "$DEPENDENCIES" ]]; then
    DEPENDENCIES="$ROOT/Dependencies"
  fi
}

setup_dependencies() {
  if [[ ! -d "$DEPENDENCIES" ]]; then
    mkdir -p "$DEPENDENCIES" || return 1
  fi
  pushd "$DEPENDENCIES" > /dev/null
  "$SCRIPT_DIR/setup.sh" || { popd > /dev/null; return 1; }
  popd > /dev/null
}

create_symlinks() {
  if [[ "$DEPENDENCIES" != "$ROOT/Dependencies" ]]; then
    if [[ ! -d "Dependencies" ]]; then
      ln -s "$DEPENDENCIES" Dependencies || return 1
    fi
  fi
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
    if [[ ! -f "webpack.config.js" ]]; then
      ln -s "$DIRECTORY/webpack.config.js" webpack.config.js
    fi
  fi
}

main "$@"

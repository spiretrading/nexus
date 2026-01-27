#!/bin/bash
set -o errexit
set -o pipefail

DALI_PATH="Dependencies/dali"
NEXUS_PATH="Dependencies/WebApi"

main() {
  resolve_paths
  parse_args "$@"
  case "$CONFIG" in
    clean)
      clean_build "clean"
      exit 0
      ;;
    reset)
      clean_build "reset"
      exit 0
      ;;
  esac
  configure
  build_dependencies
  check_node_modules
  check_build
  run_build
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

parse_args() {
  DEPENDENCIES=""
  CONFIG=""
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
        CONFIG="$1"
        shift
        ;;
    esac
  done
}

clean_build() {
  local mode="$1"
  rm -rf library
  rm -f mod_time.txt
  if [[ "$mode" == "reset" ]]; then
    rm -rf Dependencies
    rm -rf node_modules
    if [[ "$DIRECTORY" != "$ROOT" ]]; then
      rm -f package.json
      rm -f tsconfig.json
    fi
  fi
}

configure() {
  if [[ ! "$SCRIPT_DIR/package.json" -ef "./package.json" ]]; then
    cp "$SCRIPT_DIR/package.json" . > /dev/null
  fi
  if [[ ! "$SCRIPT_DIR/tsconfig.json" -ef "./tsconfig.json" ]]; then
    cp "$SCRIPT_DIR/tsconfig.json" . > /dev/null
  fi
  if [[ -n "$DEPENDENCIES" ]]; then
    "$SCRIPT_DIR/configure.sh" -DD="$DEPENDENCIES"
  else
    "$SCRIPT_DIR/configure.sh"
  fi
}

build_dependencies() {
  pushd "$DALI_PATH" > /dev/null
  ./build.sh "$@"
  popd > /dev/null
  pushd "$NEXUS_PATH" > /dev/null
  ./build.sh "$@"
  popd > /dev/null
}

md5hash() {
  if [[ $# -eq 0 ]]; then
    if command -v md5sum > /dev/null 2>&1; then
      md5sum | awk '{print $1}'
    else
      md5
    fi
  else
    if command -v md5sum > /dev/null 2>&1; then
      md5sum "$@" 2> /dev/null | awk '{print $1}'
    else
      md5 -q "$@" 2> /dev/null
    fi
  fi
}

check_node_modules() {
  UPDATE_NODE=""
  if [[ ! -d "node_modules" ]]; then
    UPDATE_NODE=1
  elif [[ ! -f "mod_time.txt" ]]; then
    UPDATE_NODE=1
  else
    local pkg_hash
    local stored_hash
    pkg_hash="$(md5hash "$DIRECTORY/package.json")"
    stored_hash="$(grep "^package.json:" mod_time.txt 2> /dev/null | cut -d: -f2 || true)"
    if [[ "$pkg_hash" != "$stored_hash" ]]; then
      UPDATE_NODE=1
    fi
  fi
  if [[ -n "$UPDATE_NODE" ]]; then
    UPDATE_BUILD=1
    npm install
  fi
}

check_build() {
  if [[ ! -d "library" ]]; then
    UPDATE_BUILD=1
  elif [[ ! -f "mod_time.txt" ]]; then
    UPDATE_BUILD=1
  else
    local current_hash
    local stored_hash
    current_hash="$(compute_source_hash)"
    stored_hash="$(grep "^source:" mod_time.txt 2> /dev/null | cut -d: -f2 || true)"
    if [[ "$current_hash" != "$stored_hash" ]]; then
      UPDATE_BUILD=1
    fi
  fi
}

compute_source_hash() {
  {
    cat "$DIRECTORY/tsconfig.json"
    if [[ -f "$DALI_PATH/mod_time.txt" ]]; then
      cat "$DALI_PATH/mod_time.txt"
    fi
    if [[ -f "$NEXUS_PATH/mod_time.txt" ]]; then
      cat "$NEXUS_PATH/mod_time.txt"
    fi
    find "$DIRECTORY/source" -type f -print0 | sort -z | xargs -0 cat 2> /dev/null
  } | md5hash
}

run_build() {
  if [[ -n "$UPDATE_BUILD" ]]; then
    rm -rf library
    npm run build
    local pkg_hash
    local source_hash
    pkg_hash="$(md5hash "$DIRECTORY/package.json")"
    source_hash="$(compute_source_hash)"
    echo "package.json:$pkg_hash" > mod_time.txt
    echo "source:$source_hash" >> mod_time.txt
  fi
}

main "$@"

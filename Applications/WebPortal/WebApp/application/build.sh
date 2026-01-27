#!/bin/bash
set -o errexit
set -o pipefail
ROOT=""
SCRIPT_DIR=""
DIRECTORY=""
DEPENDENCIES=""
CONFIG=""
UPDATE_NODE=""
UPDATE_BUILD=""
WEB_PORTAL_PATH="Dependencies/library"

main() {
  resolve_paths
  parse_args "$@"
  configure
  build_dependencies
  check_node_modules
  check_build
  run_build
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

parse_args() {
  while [[ $# -gt 0 ]]; do
    case "$1" in
      -DD=*) DEPENDENCIES="${1#*=}" ;;
      -DD)   DEPENDENCIES="$2"; shift ;;
      -D=*)  DIRECTORY="${1#*=}" ;;
      -D)    DIRECTORY="$2"; shift ;;
      clean)
        clean_build "clean"
        exit 0
        ;;
      reset)
        clean_build "reset"
        exit 0
        ;;
      *)     CONFIG="$1" ;;
    esac
    shift
  done
  if [[ -z "$CONFIG" ]]; then
    CONFIG="Release"
  fi
}

clean_build() {
  local mode="$1"
  rm -rf application
  rm -f mod_time.txt
  rm -f .build_hash.txt
  if [[ "$mode" == "reset" ]]; then
    rm -rf Dependencies
    rm -rf node_modules
  fi
}

configure() {
  if [[ -n "$DEPENDENCIES" ]]; then
    "$SCRIPT_DIR/configure.sh" -DD="$DEPENDENCIES"
  else
    "$SCRIPT_DIR/configure.sh"
  fi
}

build_dependencies() {
  pushd "$WEB_PORTAL_PATH" > /dev/null
  ./build.sh || { popd > /dev/null; return 1; }
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

compute_source_hash() {
  {
    cat "$DIRECTORY/tsconfig.json"
    cat "$DIRECTORY/webpack.config.js"
    find "$DIRECTORY/source" -type f -print0 | sort -z | xargs -0 cat 2> /dev/null
    if [[ -f "$WEB_PORTAL_PATH/mod_time.txt" ]]; then
      cat "$WEB_PORTAL_PATH/mod_time.txt"
    fi
  } | md5hash
}

check_node_modules() {
  if [[ ! -d "node_modules" ]]; then
    UPDATE_NODE=1
    return
  fi
  local current_hash stored_hash
  current_hash="$(md5hash "$DIRECTORY/package.json")"
  if [[ -f "mod_time.txt" ]]; then
    stored_hash="$(cat "mod_time.txt")"
    if [[ "$stored_hash" != "$current_hash" ]]; then
      UPDATE_NODE=1
    fi
  else
    UPDATE_NODE=1
  fi
}

check_build() {
  if [[ ! -d "application" ]]; then
    UPDATE_BUILD=1
    return
  fi
  local current_hash stored_hash
  current_hash="$(compute_source_hash)"
  if [[ -f ".build_hash.txt" ]]; then
    stored_hash="$(cat ".build_hash.txt")"
    if [[ "$current_hash" != "$stored_hash" ]]; then
      UPDATE_BUILD=1
    fi
  else
    UPDATE_BUILD=1
  fi
}

run_build() {
  if [[ "$UPDATE_NODE" == "1" ]]; then
    UPDATE_BUILD=1
    npm install || return 1
    local package_hash
    package_hash="$(md5hash "$DIRECTORY/package.json")"
    echo "$package_hash" > "mod_time.txt"
  fi
  if [[ "$UPDATE_BUILD" == "1" ]]; then
    if [[ -d "application" ]]; then
      rm -rf application/*
    fi
    if [[ "$CONFIG" == "Release" ]]; then
      export PROD_ENV=1
    fi
    node node_modules/webpack/bin/webpack.js || return 1
    if [[ "$CONFIG" == "Release" ]]; then
      unset PROD_ENV
    fi
    local source_hash
    source_hash="$(compute_source_hash)"
    echo "$source_hash" > ".build_hash.txt"
    if [[ -d "application" ]]; then
      cp -r "$SCRIPT_DIR/../resources" application/
      cp "$DIRECTORY/source/index.html" application/index.html
    fi
    if [[ -d "../../Application" ]]; then
      mkdir -p ../../Application/web_app
      cp -r application/* ../../Application/web_app/
    fi
  fi
}

main "$@"

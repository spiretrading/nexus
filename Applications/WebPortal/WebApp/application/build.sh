#!/bin/bash
set -o errexit
set -o pipefail
ROOT=""
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
  build_dependencies "$@"
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
  DIRECTORY="$(cd -P "$(dirname "$source")" >/dev/null && pwd -P)"
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
    "$DIRECTORY/configure.sh" -DD="$DEPENDENCIES"
  else
    "$DIRECTORY/configure.sh"
  fi
}

build_dependencies() {
  pushd "$WEB_PORTAL_PATH" > /dev/null
  ./build.sh "$@" || { popd > /dev/null; return 1; }
  popd > /dev/null
}

md5hash() {
  if command -v md5sum >/dev/null; then
    md5sum | cut -d" " -f1
  else
    md5 | cut -d" " -f4
  fi
}

check_node_modules() {
  if [[ ! -d "node_modules" ]]; then
    UPDATE_NODE=1
    return
  fi
  local current_hash cached_hash
  current_hash=$(md5hash < "$DIRECTORY/package.json")
  if [[ -f "mod_time.txt" ]]; then
    cached_hash=$(cat "mod_time.txt")
    if [[ "$cached_hash" != "$current_hash" ]]; then
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
  local source_hash cached_hash
  source_hash=$(find "$DIRECTORY/source" -type f -print0 | sort -z |
    xargs -0 cat | md5hash)
  source_hash+=$(md5hash < "$DIRECTORY/tsconfig.json")
  source_hash+=$(md5hash < "$DIRECTORY/webpack.config.js")
  if [[ -f "$WEB_PORTAL_PATH/mod_time.txt" ]]; then
    source_hash+=$(cat "$WEB_PORTAL_PATH/mod_time.txt")
  fi
  if [[ -f ".build_hash.txt" ]]; then
    cached_hash=$(cat ".build_hash.txt")
    if [[ "$cached_hash" != "$source_hash" ]]; then
      UPDATE_BUILD=1
    fi
  else
    UPDATE_BUILD=1
  fi
  if [[ "$UPDATE_BUILD" == "1" ]]; then
    echo "$source_hash" > ".build_hash.txt"
  fi
}

run_build() {
  if [[ "$UPDATE_NODE" == "1" ]]; then
    UPDATE_BUILD=1
    npm install --no-package-lock || return 1
    local package_hash
    package_hash=$(md5hash < "$DIRECTORY/package.json")
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
    source_hash=$(find "$DIRECTORY/source" -type f -print0 | sort -z |
      xargs -0 cat | md5hash)
    source_hash+=$(md5hash < "$DIRECTORY/tsconfig.json")
    source_hash+=$(md5hash < "$DIRECTORY/webpack.config.js")
    if [[ -f "$WEB_PORTAL_PATH/mod_time.txt" ]]; then
      source_hash+=$(cat "$WEB_PORTAL_PATH/mod_time.txt")
    fi
    echo "$source_hash" > ".build_hash.txt"
    if [[ -d "application" ]]; then
      cp -r "$DIRECTORY/../resources" application/
      cp "$DIRECTORY/source/index.html" application/index.html
    fi
    if [[ -d "../../Application" ]]; then
      mkdir -p ../../Application/web_app
      cp -r application/* ../../Application/web_app/
    fi
  fi
}

main "$@"

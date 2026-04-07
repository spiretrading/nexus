#!/bin/bash
set -o errexit
set -o pipefail
ROOT=""
DIRECTORY=""
SCRIPT_DIR=""
DEPENDENCIES=""
CONFIG=""
RUN_CMAKE=""

main() {
  resolve_paths
  create_forwarding_scripts
  parse_args "$@"
  setup_dependencies || return 1
  check_hashes || return 1
  run_cmake || return 1
  run_version
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
      -DD=*)
        DEPENDENCIES="${1#*=}"
        if [[ -z "$DEPENDENCIES" ]]; then
          echo "Error: -DD requires a path argument."
          return 1
        fi
        ;;
      -DD)
        if [[ -z "$2" || "$2" == -* ]]; then
          echo "Error: -DD requires a path argument."
          return 1
        fi
        DEPENDENCIES="$2"; shift
        ;;
      -D=*)
        DIRECTORY="${1#*=}"
        if [[ -z "$DIRECTORY" ]]; then
          echo "Error: -D requires a path argument."
          return 1
        fi
        ;;
      -D)
        if [[ -z "$2" || "$2" == -* ]]; then
          echo "Error: -D requires a path argument."
          return 1
        fi
        DIRECTORY="$2"; shift
        ;;
      *)     CONFIG="$1" ;;
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
  if [[ "$DEPENDENCIES" != "$ROOT/Dependencies" ]]; then
    if [[ -e "Dependencies" ]]; then
      rm -rf Dependencies || return 1
    fi
    ln -s "$DEPENDENCIES" Dependencies || return 1
  fi
}

md5hash() {
  if command -v md5sum >/dev/null; then
    md5sum | cut -d" " -f1
  else
    md5 | cut -d" " -f4
  fi
}

check_hashes() {
  if [[ ! -d "CMakeFiles" ]]; then
    mkdir -p CMakeFiles || return 1
    RUN_CMAKE=1
  fi
  check_config
  check_cmake_hash
  if [[ -d "$DIRECTORY/Include" ]]; then
    check_directory_hash "$DIRECTORY/Include" "CMakeFiles/hpp_hash.txt"
  fi
  if [[ -d "$DIRECTORY/Source" ]]; then
    check_directory_hash "$DIRECTORY/Source" "CMakeFiles/cpp_hash.txt"
  fi
}

check_config() {
  if [[ -f "CMakeFiles/config.txt" ]]; then
    local cached_config
    cached_config=$(< "CMakeFiles/config.txt")
    if [[ "$cached_config" != "$CONFIG" ]]; then
      RUN_CMAKE=1
    fi
  else
    RUN_CMAKE=1
  fi
  if [[ "$RUN_CMAKE" == "1" ]]; then
    echo "$CONFIG" > "CMakeFiles/config.txt"
  fi
}

check_cmake_hash() {
  local temp_file="$ROOT/temp_$$.txt"
  cat "$DIRECTORY/CMakeLists.txt" > "$temp_file"
  if [[ -d "$DIRECTORY/Config" ]]; then
    for f in "$DIRECTORY/Config"/*.cmake; do
      [[ -f "$f" ]] && cat "$f" >> "$temp_file"
    done
    find "$DIRECTORY/Config" -name "CMakeLists.txt" -type f -print0 |
      sort -z | xargs -0 cat >> "$temp_file" 2>/dev/null || true
  fi
  check_file_hash "$temp_file" "CMakeFiles/cmake_hash.txt"
}

check_file_hash() {
  local file="$1"
  local hash_file="$2"
  local current_hash
  current_hash=$(md5hash < "$file")
  rm -f "$file"
  if [[ -f "$hash_file" ]]; then
    local cached_hash
    cached_hash=$(< "$hash_file")
    if [[ "$cached_hash" != "$current_hash" ]]; then
      RUN_CMAKE=1
    fi
  else
    RUN_CMAKE=1
  fi
  if [[ "$RUN_CMAKE" == "1" ]]; then
    echo "$current_hash" > "$hash_file"
  fi
}

check_directory_hash() {
  local dir="$1"
  local hash_file="$2"
  local current_hash
  current_hash=$(find "$dir" -type f | sort | md5hash)
  if [[ -f "$hash_file" ]]; then
    local cached_hash
    cached_hash=$(< "$hash_file")
    if [[ "$cached_hash" != "$current_hash" ]]; then
      RUN_CMAKE=1
    fi
  else
    RUN_CMAKE=1
  fi
  if [[ "$RUN_CMAKE" == "1" ]]; then
    echo "$current_hash" > "$hash_file"
  fi
}

run_cmake() {
  if [[ "$RUN_CMAKE" == "1" ]]; then
    cmake -S "$DIRECTORY" -DCMAKE_BUILD_TYPE="$CONFIG" -DD="$DEPENDENCIES" ||
      return 1
  fi
}

run_version() {
  if [[ -f "$DIRECTORY/version.sh" ]]; then
    local dir_version
    dir_version=$(cd -P "$DIRECTORY" && pwd -P)/version.sh
    if [[ "$dir_version" != "$SCRIPT_DIR/version.sh" ]]; then
      "$DIRECTORY/version.sh"
    fi
  fi
}

main "$@"

#!/bin/bash
set -o errexit
set -o pipefail
DIRECTORY=""
ROOT=""
SCRIPT_DIR=""
DEPENDENCIES=""
CONFIG=""
RUN_CMAKE=""
HASH_FILES=()
HASH_VALUES=()

main() {
  resolve_paths
  parse_args "$@" || return 1
  create_forwarding_scripts || return 1
  setup_dependencies || return 1
  if [[ "${NEXUS_SKIP_CMAKE:-}" == "1" ]]; then
    run_version
    return $?
  fi
  check_hashes || return 1
  if [[ "$DIRECTORY" != "$SCRIPT_DIR" &&
      -f "$DIRECTORY/version.sh" && ! -f Version.hpp ]]; then
    RUN_CMAKE=1
  fi
  if [[ "$RUN_CMAKE" == "1" ]]; then
    generated_files begin || return 1
  fi
  local configure_error=0
  configure_build || configure_error=$?
  if [[ "$RUN_CMAKE" == "1" ]]; then
    generated_files end || return 1
  fi
  return "$configure_error"
}

configure_build() {
  run_version || return 1
  run_cmake || return 1
  commit_hashes || return 1
}

generated_files() {
  cmake -DBUILD_DIRECTORY:PATH="$ROOT" \
    -DDEPENDENCIES_DIRECTORY:PATH="$DEPENDENCIES" -DACTION="$1" \
    -P "$SCRIPT_DIR/Config/generated_files.cmake"
}

resolve_paths() {
  local source="${BASH_SOURCE[0]}"
  while [[ -h "$source" ]]; do
    local dir="$(cd -P "$(dirname "$source")" >/dev/null && pwd -P)"
    source="$(readlink "$source")"
    [[ $source != /* ]] && source="$dir/$source"
  done
  DIRECTORY="$(cd -P "$(dirname "$source")" >/dev/null && pwd -P)"
  SCRIPT_DIR="$DIRECTORY"
  ROOT="$(pwd -P)"
}

create_forwarding_scripts() {
  if [[ ! -f "build.sh" ]]; then
    printf '#!/bin/bash\nexec %q "$@"\n' "$DIRECTORY/build.sh" > build.sh ||
      return 1
    chmod +x build.sh || return 1
  fi
  if [[ ! -f "configure.sh" ]]; then
    printf '#!/bin/bash\nexec %q "$@"\n' "$DIRECTORY/configure.sh" \
      > configure.sh || return 1
    chmod +x configure.sh || return 1
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
  if [[ -z "$CONFIG" ]]; then
    if [[ -f "CMakeFiles/config.txt" ]]; then
      CONFIG=$(< "CMakeFiles/config.txt")
    else
      CONFIG="Release"
    fi
  fi
  shopt -s nocasematch
  case "$CONFIG" in
    release) CONFIG="Release" ;;
    debug) CONFIG="Debug" ;;
    relwithdebinfo) CONFIG="RelWithDebInfo" ;;
    minsizerel) CONFIG="MinSizeRel" ;;
    *)
      shopt -u nocasematch
      echo "Error: Invalid configuration \"$CONFIG\"."
      return 1
      ;;
  esac
  shopt -u nocasematch
  DIRECTORY="$(cd "$DIRECTORY" && pwd -P)" || return 1
  if [[ -z "$DEPENDENCIES" ]]; then
    DEPENDENCIES="$ROOT/Dependencies"
  fi
}

setup_dependencies() {
  if [[ ! -d "$DEPENDENCIES" ]]; then
    mkdir -p "$DEPENDENCIES" || return 1
  fi
  DEPENDENCIES="$(cd "$DEPENDENCIES" && pwd -P)" || return 1
  if [[ -e "$ROOT/Dependencies" ]] &&
      [[ ! "$ROOT/Dependencies" -ef "$DEPENDENCIES" ]] &&
      [[ ! -L "$ROOT/Dependencies" ]]; then
    echo "Error: $ROOT/Dependencies exists and is not a symbolic link."
    return 1
  fi
  if [[ "${NEXUS_SETUP_DIRECTORY:-}" != "$DEPENDENCIES" ]]; then
    cmake -DDEPENDENCIES_DIRECTORY:PATH="$DEPENDENCIES" \
      -P "$SCRIPT_DIR/Config/configure_dependencies.cmake" || return 1
  fi
  export NEXUS_SETUP_DIRECTORY="$DEPENDENCIES"
  if [[ ! "$ROOT/Dependencies" -ef "$DEPENDENCIES" ]]; then
    if [[ -L "$ROOT/Dependencies" ]]; then
      rm "$ROOT/Dependencies" || return 1
    fi
    ln -s "$DEPENDENCIES" "$ROOT/Dependencies" || return 1
  fi
}

md5hash() {
  if command -v md5sum >/dev/null; then
    md5sum | cut -d" " -f1
  else
    md5 -r | cut -d" " -f1
  fi
}

check_hashes() {
  local scripts=(CMakeFiles/clean_*.cmake)
  if [[ "$DIRECTORY" == "$SCRIPT_DIR" && ! -f "${scripts[0]}" ]]; then
    RUN_CMAKE=1
  fi
  if [[ ! -f "CMakeCache.txt" ]]; then
    RUN_CMAKE=1
  else
    local cached_config="" configuration_types="" key value
    while IFS='=' read -r key value; do
      case "$key" in
        CMAKE_BUILD_TYPE:*) cached_config="${value%$'\r'}" ;;
        CMAKE_CONFIGURATION_TYPES:*) configuration_types="${value%$'\r'}" ;;
      esac
    done < CMakeCache.txt
    if [[ -z "$configuration_types" && "$cached_config" != "$CONFIG" ]]; then
      RUN_CMAKE=1
    fi
  fi
  if [[ ! -d "CMakeFiles" ]]; then
    RUN_CMAKE=1
  fi
  check_file_hash "$CONFIG" "CMakeFiles/config.txt"
  check_file_hash "$DEPENDENCIES" "CMakeFiles/dependencies.txt"
  check_directory_hash "$DIRECTORY/Include" "CMakeFiles/hpp_hash.txt"
  check_directory_hash "$DIRECTORY/Source" "CMakeFiles/cpp_hash.txt"
  check_cmake_hash
}

check_cmake_hash() {
  local current_hash
  current_hash=$( (
    cat "$DIRECTORY/CMakeLists.txt"
    for file in "$SCRIPT_DIR/Config"/*.cmake; do
      [[ -f "$file" ]] && cat "$file"
    done
    if [[ -d "$DIRECTORY/Config" ]]; then
      for f in "$DIRECTORY/Config"/*.cmake; do
        [[ -f "$f" ]] && cat "$f"
      done
      find "$DIRECTORY/Config" -name "CMakeLists.txt" -type f | sort |
        while IFS= read -r file; do
          cat "$file" || return 1
        done
    fi
  ) | md5hash)
  check_file_hash "$current_hash" "CMakeFiles/cmake_hash.txt"
}

check_file_hash() {
  local current_hash="$1"
  local hash_file="$2"
  if [[ -f "$hash_file" ]]; then
    local cached_hash
    cached_hash=$(< "$hash_file")
    if [[ "$current_hash" != "$cached_hash" ]]; then
      RUN_CMAKE=1
    fi
  else
    RUN_CMAKE=1
  fi
  HASH_FILES+=("$hash_file")
  HASH_VALUES+=("$current_hash")
}

check_directory_hash() {
  local dir="$1"
  local hash_file="$2"
  if [[ ! -d "$dir" ]]; then
    return 0
  fi
  local current_hash
  current_hash=$(find "$dir" -type f | sort | md5hash)
  check_file_hash "$current_hash" "$hash_file"
}

run_cmake() {
  if [[ "$RUN_CMAKE" == "1" ]]; then
    rm -f CMakeFiles/cmake_hash.txt || return 1
    NEXUS_SKIP_CMAKE=1 cmake -S "$DIRECTORY" \
      -DCMAKE_BUILD_TYPE="$CONFIG" -DD="$DEPENDENCIES" || return 1
  fi
}

commit_hashes() {
  if [[ "$RUN_CMAKE" == "1" ]]; then
    local i
    for ((i = 0; i < ${#HASH_FILES[@]}; ++i)); do
      printf '%s\n' "${HASH_VALUES[i]}" > "${HASH_FILES[i]}" || return 1
    done
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

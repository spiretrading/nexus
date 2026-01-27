#!/bin/bash
set -o errexit
set -o pipefail
DIRECTORY=""
ROOT=""
DEPENDENCIES=""
CONFIG=""

main() {
  resolve_paths
  parse_args "$@"
  local config_lower="${CONFIG,,}"
  if [[ "$config_lower" == "clean" ]]; then
    clean_build "clean"
    return $?
  fi
  if [[ "$config_lower" == "reset" ]]; then
    clean_build "reset"
    return $?
  fi
  configure || return 1
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
      *)     CONFIG="$1" ;;
    esac
    shift
  done
}

clean_build() {
  local mode="$1"
  if [[ "$mode" == "reset" ]]; then
    rm -rf Dependencies
    git clean -ffxd
  else
    git clean -ffxd -e "*Dependencies*"
    rm -f "Dependencies/cache_files/nexus.txt"
  fi
}

configure() {
  if [[ -z "$CONFIG" ]]; then
    if [[ -f "CMakeFiles/config.txt" ]]; then
      CONFIG=$(cat "CMakeFiles/config.txt")
    else
      CONFIG="Release"
    fi
  fi
  local config_lower="${CONFIG,,}"
  case "$config_lower" in
    release)        CONFIG="Release" ;;
    debug)          CONFIG="Debug" ;;
    relwithdebinfo) CONFIG="RelWithDebInfo" ;;
    minsizerel)     CONFIG="MinSizeRel" ;;
    *)
      echo "Error: Invalid configuration \"$CONFIG\"."
      return 1
      ;;
  esac
  if [[ -n "$DEPENDENCIES" ]]; then
    "$DIRECTORY/configure.sh" "$CONFIG" -DD="$DEPENDENCIES"
  else
    "$DIRECTORY/configure.sh" "$CONFIG"
  fi
}

run_build() {
  local jobs
  jobs=$(get_job_count)
  cmake --build "$ROOT" --target install --config "$CONFIG" -- -j"$jobs" ||
    return 1
  echo "$CONFIG" > "CMakeFiles/config.txt"
}

get_job_count() {
  local cores mem jobs
  if [[ -f /proc/cpuinfo ]]; then
    cores=$(grep -c "processor" /proc/cpuinfo)
  else
    cores=$(sysctl -n hw.ncpu 2>/dev/null || echo 4)
  fi
  if [[ -f /proc/meminfo ]]; then
    mem=$(awk '/MemTotal/ {print int($2 / 4194304)}' /proc/meminfo)
  else
    mem=$(sysctl -n hw.memsize 2>/dev/null |
      awk '{print int($1 / 4294967296)}' || echo 4)
  fi
  ((cores -= 2))
  [[ $cores -lt 1 ]] && cores=1
  [[ $mem -lt 1 ]] && mem=1
  jobs=$((cores < mem ? cores : mem))
  echo "$jobs"
}

main "$@"

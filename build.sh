#!/bin/bash
set -o errexit
set -o pipefail
DIRECTORY=""
ROOT=""
DEPENDENCIES=""
ARGS=()
CONFIG=""

main() {
  resolve_paths
  parse_args "$@" || return 1
  create_forwarding_scripts
  local targets=(
    "Applications/AdministrationServer"
    "Applications/ChartingServer"
    "Applications/ComplianceServer"
    "Applications/DefinitionsServer"
    "Applications/MarketDataRelayServer"
    "Applications/MarketDataServer"
    "Applications/ReplayMarketDataFeedClient"
    "Applications/RiskServer"
    "Applications/Scratch"
    "Applications/SimulationMarketDataFeedClient"
    "Applications/SimulationOrderExecutionServer"
    "Applications/WebPortal/WebApp"
    "Applications/WebPortal"
  )
  shopt -s nocasematch
  if [[ "$CONFIG" == "clean" || "$CONFIG" == "reset" ]]; then
    shopt -u nocasematch
    local status=0
    for target in "WebApi" "${targets[@]}"; do
      build_function "${ARGS[@]}" "$target" || status=1
    done
    if [[ "$status" == "0" ]]; then
      build_function "${ARGS[@]}" "Nexus" || status=1
    fi
    return "$status"
  fi
  shopt -u nocasematch
  build_function "${ARGS[@]}" "Nexus"
  build_function "${ARGS[@]}" "WebApi"
  local jobs
  jobs=$(get_job_count)
  export -f build_function
  export DIRECTORY
  export ROOT
  export DEPENDENCIES
  parallel -j"$jobs" --no-notice --quote build_function "${ARGS[@]}" ::: \
    "${targets[@]}"
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
  DEPENDENCIES="$ROOT/Nexus/Dependencies"
  ARGS=()
  while [[ $# -gt 0 ]]; do
    local arg="$1"
    if [[ "$arg" == "-DD" ]]; then
      shift
      if [[ $# -eq 0 || -z "$1" ]]; then
        echo "Error: -DD requires a path argument."
        return 1
      fi
      DEPENDENCIES="$1"
    elif [[ "$arg" == -DD=* ]]; then
      DEPENDENCIES="${arg#-DD=}"
      if [[ -z "$DEPENDENCIES" ]]; then
        echo "Error: -DD requires a path argument."
        return 1
      fi
    else
      ARGS+=("$arg")
      CONFIG="$arg"
    fi
    shift
  done
  if [[ "$DEPENDENCIES" != /* ]]; then
    DEPENDENCIES="$ROOT/$DEPENDENCIES"
  fi
}

create_forwarding_scripts() {
  if [[ ! -f "configure.sh" ]]; then
    ln -s "$DIRECTORY/configure.sh" configure.sh
  fi
  if [[ ! -f "build.sh" ]]; then
    ln -s "$DIRECTORY/build.sh" build.sh
  fi
}

build_function() {
  local location="${*: -1}"
  if [[ ! -d "$location" ]]; then
    mkdir -p "$location" || return 1
  fi
  pushd "$location" > /dev/null || return 1
  local status=0
  "$DIRECTORY/$location/build.sh" -DD="$DEPENDENCIES" \
    "${@:1:$#-1}" || status=$?
  popd > /dev/null || return 1
  return "$status"
}

get_job_count() {
  local cores mem jobs
  if [[ -f /proc/cpuinfo ]]; then
    cores=$(grep -c "processor" /proc/cpuinfo)
  else
    cores=$(sysctl -n hw.ncpu 2>/dev/null || echo 4)
  fi
  if [[ -f /proc/meminfo ]]; then
    mem=$(awk '/MemTotal/ {print int($2 / 8388608)}' /proc/meminfo)
  else
    mem=$(sysctl -n hw.memsize 2>/dev/null |
      awk '{print int($1 / 8589934592)}' || echo 4)
  fi
  ((cores -= 2))
  [[ $cores -lt 1 ]] && cores=1
  [[ $mem -lt 1 ]] && mem=1
  jobs=$((cores < mem ? cores : mem))
  echo "$jobs"
}

main "$@"

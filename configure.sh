#!/bin/bash
set -o errexit
set -o pipefail
DIRECTORY=""
ROOT=""
DEPENDENCIES=""
ARGS=()

main() {
  resolve_paths
  parse_args "$@" || return 1
  create_forwarding_scripts
  unset NEXUS_SETUP_DIRECTORY
  configure_target Nexus "${ARGS[@]}"
  NEXUS_SETUP_DIRECTORY="$(cd "$DEPENDENCIES" && pwd -P)" || return 1
  export NEXUS_SETUP_DIRECTORY
  local targets=(
    "WebApi"
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
  for target in "${targets[@]}"; do
    configure_target "$target" "${ARGS[@]}"
  done
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

configure_target() {
  local target="$1"
  shift
  if [[ ! -d "$target" ]]; then
    mkdir -p "$target"
  fi
  pushd "$target" > /dev/null
  "$DIRECTORY/$target/configure.sh" -DD="$DEPENDENCIES" "$@"
  popd > /dev/null
}

main "$@"

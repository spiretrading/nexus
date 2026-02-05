#!/bin/bash
set -o errexit
set -o pipefail
DIRECTORY=""
ROOT=""

main() {
  resolve_paths
  create_forwarding_scripts
  local targets=(
    "Nexus"
    "WebApi"
    "Applications/AdministrationServer"
    "Applications/AsterMarketDataFeedClient"
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
    configure_target "$target" "$@"
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
  "$DIRECTORY/$target/configure.sh" -DD="$ROOT/Nexus/Dependencies" "$@"
  popd > /dev/null
}

main "$@"

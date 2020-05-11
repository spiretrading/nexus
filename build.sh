#!/bin/bash
source="${BASH_SOURCE[0]}"
while [ -h "$source" ]; do
  dir="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd)"
  source="$(readlink "$source")"
  [[ $source != /* ]] && source="$dir/$source"
done
directory="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd)"
root=$(pwd)
build_function() {
  location="${@: -1}"
  if [ ! -d "$location" ]; then
    mkdir -p "$location"
  fi
  pushd "$location"
  "$directory/$location/build.sh" "${@:1:$#-1}"
  popd
}

export -f build_function
export directory

build_function "$@" "Nexus"
build_function "$@" "WebApi"
targets+=" Applications/AdministrationServer"
targets+=" Applications/ChartingServer"
targets+=" Applications/ComplianceServer"
targets+=" Applications/DefinitionsServer"
targets+=" Applications/MarketDataRelayServer"
targets+=" Applications/MarketDataServer"
targets+=" Applications/ReplayMarketDataFeedClient"
targets+=" Applications/RiskServer"
targets+=" Applications/SimulationMarketDataFeedClient"
targets+=" Applications/SimulationOrderExecutionServer"
targets+=" Applications/WebPortal"
targets+=" Applications/WebPortal/WebApp"

let cores="`grep -c "processor" < /proc/cpuinfo` / 2 + 1"
let mem="`grep -oP "MemTotal: +\K([[:digit:]]+)(?=.*)" < /proc/meminfo` / 4194304"
let jobs="$(($cores<$mem?$cores:$mem))"

parallel -j$jobs --no-notice build_function "$@" ::: $targets

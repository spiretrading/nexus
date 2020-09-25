#!/bin/bash
source="${BASH_SOURCE[0]}"
while [ -h "$source" ]; do
  dir="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd -P)"
  source="$(readlink "$source")"
  [[ $source != /* ]] && source="$dir/$source"
done
directory="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd -P)"
root=$(pwd -P)
if [ ! -f "configure.sh" ]; then
  ln -s "$directory/configure.sh" configure.sh
fi
if [ ! -f "build.sh" ]; then
  ln -s "$directory/build.sh" build.sh
fi
build_function() {
  location="${@: -1}"
  if [ ! -d "$location" ]; then
    mkdir -p "$location"
  fi
  pushd "$location"
  "$directory/$location/build.sh" -DD="$root/Nexus/Dependencies" "${@:1:$#-1}" 2>&1
  popd
}

export -f build_function
export directory
export root

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
targets+=" Applications/Scratch"
targets+=" Applications/SimulationMarketDataFeedClient"
targets+=" Applications/SimulationOrderExecutionServer"
targets+=" Applications/WebPortal/WebApp"
targets+=" Applications/WebPortal"

cores="`grep -c "processor" < /proc/cpuinfo` - 2"
mem="`grep -oP "MemTotal: +\K([[:digit:]]+)(?=.*)" < /proc/meminfo` / 8388608"
jobs="$(($cores<$mem?$cores:$mem))"
parallel -j$jobs --no-notice build_function "$@" ::: $targets

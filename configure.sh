#!/bin/bash
source="${BASH_SOURCE[0]}"
while [ -h "$source" ]; do
  dir="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd)"
  source="$(readlink "$source")"
  [[ $source != /* ]] && source="$dir/$source"
done
directory="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd)"
root="$(pwd)"
if [ ! -f "configure.sh" ]; then
  ln -s "$directory/configure.sh" configure.sh
fi
if [ ! -f "build.sh" ]; then
  ln -s "$directory/build.sh" build.sh
fi
targets="Nexus"
targets+=" WebApi"
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

for i in $targets; do
  if [ ! -d "$i" ]; then
    mkdir -p "$i"
  fi
  pushd "$i"
  "$directory/$i/configure.sh" -DD="$root/Nexus/Dependencies" "$@"
  popd
done

#!/bin/bash
set -o errexit
set -o pipefail
directory=$(pwd)
if [ "$1" = "" ]
then
  config="install"
else
  config=$1
fi

build_function() {
  pushd $directory/../../Applications/$1/Build/Make
  ./build.sh $config
  popd
}

let cores="`grep -c "processor" < /proc/cpuinfo` / 2 + 1"
let mem="`grep -oP "MemTotal: +\K([[:digit:]]+)(?=.*)" < /proc/meminfo` / 4194304"
let jobs="$(($cores<$mem?$cores:$mem))"

pushd $directory/../../Nexus/Build/Make
./build.sh $config
popd
export -f build_function
export directory
export config
applications=""
applications+="AdministrationServer "
applications+="AsxItchMarketDataFeedClient "
applications+="ChartingServer "
applications+="ChiaMarketDataFeedClient "
applications+="ClientWebPortal "
applications+="ComplianceServer "
applications+="CseMarketDataFeedClient "
applications+="CtaMarketDataFeedClient "
applications+="DefinitionsServer "
applications+="MarketDataClientStressTest "
applications+="MarketDataClientTemplate "
applications+="MarketDataRelayServer "
applications+="MarketDataServer "
applications+="OrderExecutionBackup "
applications+="OrderExecutionRestore "
applications+="ReplayMarketDataFeedClient "
applications+="RiskServer "
applications+="SimulationMarketDataFeedClient "
applications+="SimulationOrderExecutionServer "
applications+="TmxIpMarketDataFeedClient "
applications+="TmxTl1MarketDataFeedClient "
applications+="UtpMarketDataFeedClient "
parallel -j$jobs --no-notice build_function ::: $applications

pushd $directory/../../Applications/ClientWebPortal/Source/api/build/make
./build.sh $config
popd
pushd $directory/../../Applications/ClientWebPortal/Source/webapp/build/make
./build.sh $config
popd

pushd $directory/../../Documents/sphinx
make clean
make html
popd

#!/bin/bash
set -o errexit
set -o pipefail
directory=$(dirname $(readlink -f $0))
if [ "$1" = "" ]
then
  build_type="Release"
else
  build_type=$1
fi
pushd $directory/../../Nexus/Build/Make/
./run_cmake.sh $build_type
popd

applications=""
applications+="AdministrationServer "
applications+="AsxItchMarketDataFeedClient "
applications+="ChartingServer "
applications+="ChiaMarketDataFeedClient "
applications+="ComplianceServer "
applications+="CseMarketDataFeedClient "
applications+="CtaMarketDataFeedClient "
applications+="DefinitionsServer "
applications+="MarketDataRelayServer "
applications+="MarketDataServer "
applications+="RiskServer "
applications+="SimulationMarketDataFeedClient "
applications+="SimulationOrderExecutionServer "
applications+="TmxIpMarketDataFeedClient "
applications+="TmxTl1MarketDataFeedClient "
applications+="UtpMarketDataFeedClient "

for i in $applications; do
  pushd $directory/../../Applications/$i/Build/Make
  ./run_cmake.sh $build_type
  popd
done
pushd $directory/../../Applications/spire/build/posix
./run_cmake.sh $build_type
popd
pushd $directory/../../Applications/web_portal/build/posix
./run_cmake.sh $build_type
popd

#!/bin/bash
services="SimulationMarketDataFeedClient"
services+=" AsxItchMarketDataFeedClient"
services+=" ChiaMarketDataFeedClient"
services+=" CseMarketDataFeedClient"
services+=" NeoeMarketDataFeedClient"
services+=" TmxTl1MarketDataFeedClient"
services+=" TmxIpMarketDataFeedClient"
services+=" WebPortal"
services+=" RiskServer"
services+=" SimulationOrderExecutionServer"
services+=" OasisOrderExecutionServer"
services+=" ComplianceServer"
services+=" ChartingServer"
services+=" MarketDataRelayServer"
services+=" MarketDataServer"
services+=" AdministrationServer"
services+=" DefinitionsServer"
services+=" UidServer"
services+=" ServiceLocator"

for directory in $services; do
  pushd $directory/Application > /dev/null
  ./stop.sh
  popd > /dev/null
done

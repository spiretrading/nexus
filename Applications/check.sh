#!/bin/bash
services="ServiceLocator"
services+=" UidServer"
services+=" DefinitionsServer"
services+=" AdministrationServer"
services+=" MarketDataServer"
services+=" MarketDataRelayServer"
services+=" ChartingServer"
services+=" ComplianceServer"
services+=" OasisOrderExecutionServer"
services+=" SimulationOrderExecutionServer"
services+=" RiskServer"
services+=" TelemetryServer"
services+=" WebPortal"
services+=" SimulationMarketDataFeedClient"
services+=" AsxItchMarketDataFeedClient"
services+=" ChiaMarketDataFeedClient"
services+=" CseMarketDataFeedClient"
services+=" NeoeMarketDataFeedClient"
services+=" TmxTl1MarketDataFeedClient"
services+=" TmxIpMarketDataFeedClient"

for directory in $services; do
  pushd $directory/Application > /dev/null
  ./check.sh "$@"
  popd > /dev/null
done

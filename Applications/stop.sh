#!/bin/bash
services=" SimulationMarketDataFeedClient"
services+=" WebPortal"
services+=" TelemetryServer"
services+=" RiskServer"
services+=" SimulationOrderExecutionServer"
services+=" ComplianceServer"
services+=" ChartingServer"
services+=" MarketDataRelayServer"
services+=" MarketDataServer"
services+=" AdministrationServer"
services+=" DefinitionsServer"
services+=" RegistryServer"
services+=" UidServer"
services+=" ServiceLocator"
feeds="AsxItchMarketDataFeedClient"
feeds+=" ChiaMarketDataFeedClient"
feeds+=" CseMarketDataFeedClient"
feeds+=" CtaMarketDataFeedClient"
feeds+=" HkexMarketDataFeedClient"
feeds+=" JpxFlexMarketDataFeedClient"
feeds+=" NeoeMarketDataFeedClient"
feeds+=" TmxTl1MarketDataFeedClient"
feeds+=" TmxIpMarketDataFeedClient"
feeds+=" UtpMarketDataFeedClient"

for directory in $services; do
  pushd $directory/Application > /dev/null
  ./stop_server.sh
  popd > /dev/null
done
for directory in $feeds; do
  pushd $directory/Application > /dev/null
  ./stop_feed.sh all
  popd > /dev/null
done

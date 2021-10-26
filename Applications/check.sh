#!/bin/bash
services="ServiceLocator"
services+=" UidServer"
services+=" RegistryServer"
services+=" DefinitionsServer"
services+=" AdministrationServer"
services+=" MarketDataServer"
services+=" MarketDataRelayServer"
services+=" ChartingServer"
services+=" ComplianceServer"
services+=" SimulationMarketDataFeedClient"
services+=" SimulationOrderExecutionServer"
services+=" RiskServer"
services+=" TelemetryServer"
services+=" WebPortal"
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
  ./check_server.sh
  popd > /dev/null
done
for directory in $feeds; do
  pushd $directory
  ./list_feeds.sh
  popd
done

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
services+=" SimulationOrderExecutionServer"
services+=" RiskServer"
services+=" SimulationMarketDataFeedClient"
services+=" WebPortal"
feeds="AsxItchMarketDataFeedClient"
feeds+=" ChiaMarketDataFeedClient"
feeds+=" CseMarketDataFeedClient"
feeds+=" CtaMarketDataFeedClient"
feeds+=" HkexMarketDataFeedClient"
feeds+=" JpxFlexMarketDataFeedClient"
feeds+=" TmxTl1MarketDataFeedClient"
feeds+=" TmxIpMarketDataFeedClient"
feeds+=" UtpMarketDataFeedClient"

for directory in $services; do
  pushd $directory
  ./check_server.sh
  popd
done
for directory in $feeds; do
  cd $directory
  ./list_feeds.sh
  cd ..
done

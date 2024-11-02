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
services+=" TelemetryServer"
services+=" WebPortal"
services+=" SimulationMarketDataFeedClient"
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
  ./start.sh
  popd > /dev/null
done
for directory in $feeds; do
  pushd $directory/Application > /dev/null
  ./start_feed.sh all
  popd > /dev/null
done

pushd AdministrationServer/Application > /dev/null
python3 reset_risk_states.py
popd > /dev/null

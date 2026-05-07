#!/bin/bash
services=(
  "OtcLinkMarketDataFeedClient"
  "AsxItchMarketDataFeedClient"
  "ChiaMarketDataFeedClient"
  "CseMarketDataFeedClient"
  "NeoeMarketDataFeedClient"
  "TmxTl1MarketDataFeedClient"
  "TmxIpMarketDataFeedClient"
  "WebPortal"
  "RiskServer"
  "SimulationOrderExecutionServer"
  "ComplianceServer"
  "ChartingServer"
  "MarketDataRelayServer"
  "MarketDataServer"
  "AdministrationServer"
  "DefinitionsServer"
  "UidServer"
  "ServiceLocator"
)

for directory in "${services[@]}"; do
  pushd $directory/Application > /dev/null
  ./stop.sh
  popd > /dev/null
done

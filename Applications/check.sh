#!/bin/bash
services=(
  "ServiceLocator"
  "UidServer"
  "DefinitionsServer"
  "AdministrationServer"
  "MarketDataServer"
  "MarketDataRelayServer"
  "ChartingServer"
  "ComplianceServer"
  "OasisOrderExecutionServer"
  "SimulationOrderExecutionServer"
  "RiskServer"
  "WebPortal"
  "SimulationMarketDataFeedClient"
  "AsxItchMarketDataFeedClient"
  "ChiaMarketDataFeedClient"
  "CseMarketDataFeedClient"
  "NeoeMarketDataFeedClient"
  "TmxTl1MarketDataFeedClient"
  "TmxIpMarketDataFeedClient"
)

for directory in "${services[@]}"; do
  pushd $directory/Application > /dev/null
  ./check.sh "$@"
  popd > /dev/null
done

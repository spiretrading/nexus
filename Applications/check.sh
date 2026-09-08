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
  "CxaPitchMarketDataFeedClient"
  "NeoeMarketDataFeedClient"
  "TmxTl1MarketDataFeedClient"
  "TmxIpMarketDataFeedClient"
  "OtcLinkMarketDataFeedClient"
)

for directory in "${services[@]}"; do
  pushd $directory/Application > /dev/null
  ./check.sh "$@"
  popd > /dev/null
done

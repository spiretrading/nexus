#!/bin/bash
services=(
  "SimulationMarketDataFeedClient"
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

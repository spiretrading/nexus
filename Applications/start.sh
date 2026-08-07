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
  "SimulationOrderExecutionServer"
  "RiskServer"
  "WebPortal"
  "SimulationMarketDataFeedClient"
)

for directory in "${services[@]}"; do
  pushd $directory/Application > /dev/null
  ./start.sh
  popd > /dev/null
done

pushd AdministrationServer/Application > /dev/null
python3 reset_risk_states.py
popd > /dev/null

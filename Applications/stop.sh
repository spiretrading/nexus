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

status=0
for directory in "${services[@]}"; do
  application="$directory/Application"
  if [[ ! -d "$application" ]]; then
    application="$directory"
  fi
  (cd "$application" && ./stop.sh "$@") || status=$?
done
exit "$status"

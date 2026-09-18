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

status=0
for directory in "${services[@]}"; do
  application="$directory/Application"
  if [[ ! -d "$application" ]]; then
    application="$directory"
  fi
  (cd "$application" && ./start.sh "$@") || exit $?
done

application="AdministrationServer/Application"
if [[ ! -d "$application" ]]; then
  application="AdministrationServer"
fi
(cd "$application" && python3 reset_risk_states.py) || exit $?
exit "$status"

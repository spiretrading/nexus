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
  "AsxTradeItchMarketDataFeedClient"
  "CseMarketDataFeedClient"
  "CxaPitchMarketDataFeedClient"
  "NeoeMarketDataFeedClient"
  "TmxTl1MarketDataFeedClient"
  "TmxIpMarketDataFeedClient"
  "OtcLinkMarketDataFeedClient"
)

status=0
for directory in "${services[@]}"; do
  application="$directory/Application"
  if [[ ! -d "$application" ]]; then
    application="$directory"
  fi
  (cd "$application" && ./check.sh "$@") || status=$?
done
exit "$status"

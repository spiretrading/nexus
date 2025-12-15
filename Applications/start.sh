#!/bin/bash
services="ServiceLocator"
services+=" UidServer"
services+=" DefinitionsServer"
services+=" AdministrationServer"
services+=" MarketDataServer"
services+=" MarketDataRelayServer"
services+=" ChartingServer"
services+=" ComplianceServer"
services+=" SimulationOrderExecutionServer"
services+=" RiskServer"
services+=" WebPortal"
services+=" SimulationMarketDataFeedClient"
services+=" AsxItchMarketDataFeedClient"
services+=" ChiaMarketDataFeedClient"
services+=" CseMarketDataFeedClient"
services+=" NeoeMarketDataFeedClient"
services+=" TmxTl1MarketDataFeedClient"
services+=" TmxIpMarketDataFeedClient"

for directory in $services; do
  pushd $directory/Application > /dev/null
  ./start.sh
  popd > /dev/null
done

pushd AdministrationServer/Application > /dev/null
python3 reset_risk_states.py
popd > /dev/null

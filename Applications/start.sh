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
services+=" WebPortal"
services+=" SimulationMarketDataFeedClient"

for directory in $services; do
  pushd $directory/Application
  ./stop_server.sh
  ./start_server.sh
  popd
  sleep 3
done

pushd AdministrationServer/Application
python3 reset_risk_states.py
popd

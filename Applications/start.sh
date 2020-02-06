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
services+=" SimulationMarketDataFeedClient"
services+=" WebPortal"

for directory in $services; do
  sleep 5
  pushd $directory
  ./stop_server.sh
  ./start_server.sh
  popd
done

pushd AdministrationServer
python3 reset_risk_states.py
popd

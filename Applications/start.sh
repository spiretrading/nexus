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
services+=" TelemetryServer"
services+=" WebPortal"
services+=" SimulationMarketDataFeedClient"

for directory in $services; do
  pushd $directory/Application > /dev/null
  ./start.sh
  popd > /dev/null
  sleep 3
done

pushd AdministrationServer/Application > /dev/null
python3 reset_risk_states.py
popd > /dev/null

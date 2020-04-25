#!/bin/bash
services=" SimulationMarketDataFeedClient"
services+=" WebPortal"
services+=" RiskServer"
services+=" SimulationOrderExecutionServer"
services+=" ComplianceServer"
services+=" ChartingServer"
services+=" MarketDataRelayServer"
services+=" MarketDataServer"
services+=" AdministrationServer"
services+=" DefinitionsServer"
services+=" RegistryServer"
services+=" UidServer"
services+=" ServiceLocator"

for directory in $services; do
  pushd $directory/Application
  ./stop_server.sh
  popd
  sleep 3
done

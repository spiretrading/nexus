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
  pushd $directory/Application > /dev/null
  ./stop_server.sh
  popd > /dev/null
  sleep 1
done

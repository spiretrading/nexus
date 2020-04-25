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
  ./check_server.sh
  popd
done

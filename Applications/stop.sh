#!/bin/bash
services=" SimulationMarketDataFeedClient"
services+=" WebPortal"
services+=" TelemetryServer"
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
  ./stop.sh
  popd > /dev/null
done

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
services+=" TelemetryServer"

for directory in $services; do
  pushd $directory/Application > /dev/null
  ./check.sh
  popd > /dev/null
done

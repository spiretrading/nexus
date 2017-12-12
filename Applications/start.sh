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
services+=" ClientWebPortal"

username=$(egrep "username: (.*)" < ./RiskServer/config.yml | egrep -o ": (.*)" | egrep -o "[^: ]+(.*)")
password=$(egrep "password: (.*)" < ./RiskServer/config.yml | egrep -o ": (.*)" | egrep -o "[^: ]+(.*)")
mysql -h 127.0.0.1 -u$username -p$password -Dspire < ./reset.sql

for directory in $services; do
  sleep 10
  cd $directory
  ./stop_server.sh
  ./start_server.sh
  cd ..
done

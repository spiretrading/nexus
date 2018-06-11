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
services+=" client_web_portal"

hostname=$(egrep -A10 "data_store:" < \
  ./AdministrationServer/config.yml | \
  egrep "address:\s*([^:]*)" | \
  head -1 | \
  egrep -o ":\s*([^:]*)" | 
  head -1 | \
  egrep -o "[^: ]+(.*)")
username=$(egrep -A10 "data_store:" < \
  ./AdministrationServer/config.yml | \
  egrep "username:\s*(.*)" | \
  head -1 | \
  egrep -o ":\s*(.*)" | \
  egrep -o "[^: ]+(.*)")
password=$(egrep -A10 "data_store:" < \
  ./AdministrationServer/config.yml | \
  egrep "password:(\s|\")*([^\"]*)" | \
  head -1 | \
  egrep -o ":(\s|\")*([^\"]*)" | \
  egrep -o "[^: \"]+(.*)")
mysql -h $hostname -u$username -p$password -Dspire < ./reset.sql

for directory in $services; do
  sleep 5
  cd $directory
  ./stop_server.sh
  ./start_server.sh
  cd ..
done

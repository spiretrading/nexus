#!/bin/bash
beam_applications="AdminClient"
beam_applications+=" ServiceLocator"
beam_applications+=" UidServer"
beam_applications+=" RegistryServer"
nexus_applications="DefinitionsServer"
nexus_applications+=" AdministrationServer"
nexus_applications+=" MarketDataServer"
nexus_applications+=" MarketDataRelayServer"
nexus_applications+=" ChartingServer"
nexus_applications+=" ComplianceServer"
nexus_applications+=" RiskServer"
nexus_applications+=" SimulationOrderExecutionServer"
nexus_applications+=" SimulationMarketDataFeedClient"

for application in $beam_applications; do
  if [ ! -d "$application" ]; then
    mkdir -p $application
    cp Nexus/Dependencies/Beam/Applications/$application/Application/*.sh $application
    cp Nexus/Dependencies/Beam/Applications/$application/Application/*.default.yml $application/config.yml
  fi
  cp Nexus/Dependencies/Beam/Applications/$application/Application/$application $application
  cp Nexus/Dependencies/Beam/Applications/$application/Application/*.py $application
done
for application in $nexus_applications; do
  if [ ! -d "$application" ]; then
    mkdir -p $application
    cp Nexus/Applications/$application/Application/*.sh $application
    cp Nexus/Applications/$application/Application/*.default.yml $application/config.yml
  fi
  cp Nexus/Applications/$application/Application/$application $application
  cp Nexus/Applications/$application/Application/*.py $application
done

python_directory=$(python3 -m site --user-site)
cp Nexus/Applications/WebPortal/Application/WebPortal WebPortal
cp -R Nexus/Applications/WebPortal/Application/web_app WebPortal
cp Nexus/Dependencies/aspen/Libraries/Release/aspen.so $python_directory
mkdir -p $python_directory/beam
cp Nexus/Dependencies/Beam/Applications/Python/__init__.py $python_directory/beam
cp Nexus/Dependencies/Beam/Beam/Libraries/Release/_beam.so $python_directory/beam
mkdir -p $python_directory/nexus
cp Nexus/Applications/Python/__init__.py $python_directory/nexus
cp Nexus/Nexus/Libraries/Release/_nexus.so $python_directory/nexus

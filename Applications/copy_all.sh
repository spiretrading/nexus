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
nexus_applications+=" web_portal"

for application in $beam_applications; do
  pushd $application
  cp /home/developers/Beam/Applications/$application/Application/$application .
  popd
done
for application in $nexus_applications; do
  pushd $application
  cp /home/developers/Nexus/Applications/$application/Application/$application .
  popd
done

python_directory=$(python3 -m site --user-site)
cp -R /home/developers/Nexus/Applications/web_portal/application/web_app ./web_portal
cp /home/developers/Beam/Beam/Library/Release/beam.so $python_directory
cp /home/developers/Nexus/Nexus/Library/Release/nexus.so $python_directory

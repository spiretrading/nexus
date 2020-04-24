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
nexus_applications+=" WebPortal"

make_copy() {
  cp_path="$1"
  mkdir -p "$application"
  if [ ! -f "$application/config.yml" ]; then
    cp "$cp_path/config.default.yml" "$application" 2>/dev/null || :
  fi
  for name in $(ls "$cp_path" | grep ".*\.default\.yml" | grep -v "config.*" | \
      sed "s/\.default\.yml//"); do
    if [ ! -f "$application/$name.yml" ]; then
      cp "$cp_path/$name.default.yml" "$application/$name.yml"
    fi
  done
  cp "$cp_path/"*.sh "$application" 2>/dev/null || :
  cp "$cp_path/$application" "$application"
  cp "$cp_path/"*.csv "$application" 2>/dev/null || :
  cp "$cp_path/"*.py "$application" 2>/dev/null || :
}

for application in $beam_applications; do
  path="Nexus/Dependencies/Beam/Applications/$application/Application"
  make_copy "$path"
done
for application in $nexus_applications; do
  path="Nexus/Applications/$application/Application"
  make_copy "$path"
done

cp -R Nexus/Applications/WebPortal/Application/web_app WebPortal
./install_python.sh

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
feed_applications="AsxItchMarketDataFeedClient"
feed_applications+=" ChiaMarketDataFeedClient"
feed_applications+=" CseMarketDataFeedClient"
feed_applications+=" CtaMarketDataFeedClient"
feed_applications+=" TmxIpMarketDataFeedClient"
feed_applications+=" TmxTl1MarketDataFeedClient"
feed_applications+=" UtpMarketDataFeedClient"

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
if [ "$1" = "-f" ]; then
  for application in $feed_applications; do
    path="Nexus/Applications/$application/Application"
    make_copy "$path"
    pushd "$application"
    for sub_feed in $(ls -d "../$path/"*/ | xargs -L 1 basename); do
      if [ ! -d "$sub_feed" ]; then
        cp -r "../$path/$sub_feed" .
      else
        if [ ! -f "$sub_feed/config.yml" ]; then
          cp "../$path/$sub_feed/config.default.yml" "$sub_feed"
        fi
      fi
    done
    ./create_softlinks.sh
    popd
  done
fi

python_directory=$(python3 -m site --user-site)
cp -R Nexus/Applications/WebPortal/Application/web_app WebPortal
cp Nexus/Dependencies/aspen/Libraries/Release/aspen.so $python_directory
mkdir -p $python_directory/beam
cp Nexus/Dependencies/Beam/Applications/Python/__init__.py \
  $python_directory/beam
cp Nexus/Dependencies/Beam/Beam/Libraries/Release/_beam.so \
  $python_directory/beam
mkdir -p $python_directory/nexus
cp Nexus/Applications/Python/__init__.py $python_directory/nexus
cp Nexus/Nexus/Libraries/Release/_nexus.so $python_directory/nexus

#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/SimulationMarketDataFeedClient
printf "#define SIMULATION_MARKET_DATA_FEED_CLIENT_VERSION \""> ./../../Include/SimulationMarketDataFeedClient/Version.hpp
hg id -n | tr -d "\n" >> ./../../Include/SimulationMarketDataFeedClient/Version.hpp
printf \" >> ./../../Include/SimulationMarketDataFeedClient/Version.hpp

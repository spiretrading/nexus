#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/SimulationMarketDataFeedClient
printf "#define SIMULATION_MARKET_DATA_FEED_CLIENT_VERSION \""> ./../../Include/SimulationMarketDataFeedClient/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/SimulationMarketDataFeedClient/Version.hpp
printf \""\n" >> ./../../Include/SimulationMarketDataFeedClient/Version.hpp

#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/MarketDataRelayServer
printf "#define MARKET_DATA_RELAY_SERVER_VERSION \""> ./../../Include/MarketDataRelayServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/MarketDataRelayServer/Version.hpp
printf \""\n" >> ./../../Include/MarketDataRelayServer/Version.hpp

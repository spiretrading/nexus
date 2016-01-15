#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/MarketDataRelayServer
printf "#define MARKET_DATA_RELAY_SERVER_VERSION \""> ./../../Include/MarketDataRelayServer/Version.hpp
hg id -n | tr -d "\n" >> ./../../Include/MarketDataRelayServer/Version.hpp
printf \" >> ./../../Include/MarketDataRelayServer/Version.hpp

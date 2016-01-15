#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/MarketDataServer
printf "#define MARKET_DATA_SERVER_VERSION \""> ./../../Include/MarketDataServer/Version.hpp
hg id -n | tr -d "\n" >> ./../../Include/MarketDataServer/Version.hpp
printf \" >> ./../../Include/MarketDataServer/Version.hpp

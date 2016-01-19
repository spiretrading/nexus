#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/MarketDataServer
printf "#define MARKET_DATA_SERVER_VERSION \""> ./../../Include/MarketDataServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/MarketDataServer/Version.hpp
printf \""\n" >> ./../../Include/MarketDataServer/Version.hpp

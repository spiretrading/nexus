#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/AsxItchMarketDataFeedClient
printf "#define ASX_ITCH_MARKET_DATA_FEED_CLIENT_VERSION \""> ./../../Include/AsxItchMarketDataFeedClient/Version.hpp
hg id -n | tr -d "\n" >> ./../../Include/AsxItchMarketDataFeedClient/Version.hpp
printf \" >> ./../../Include/AsxItchMarketDataFeedClient/Version.hpp

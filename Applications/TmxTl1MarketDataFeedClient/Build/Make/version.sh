#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/TmxTl1MarketDataFeedClient
printf "#define TMX_TL1_MARKET_DATA_FEED_CLIENT_VERSION \""> ./../../Include/TmxTl1MarketDataFeedClient/Version.hpp
hg id -n | tr -d "\n" >> ./../../Include/TmxTl1MarketDataFeedClient/Version.hpp
printf \" >> ./../../Include/TmxTl1MarketDataFeedClient/Version.hpp

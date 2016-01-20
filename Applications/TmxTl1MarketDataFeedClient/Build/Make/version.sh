#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/TmxTl1MarketDataFeedClient
printf "#define TMX_TL1_MARKET_DATA_FEED_CLIENT_VERSION \""> ./../../Include/TmxTl1MarketDataFeedClient/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/TmxTl1MarketDataFeedClient/Version.hpp
printf \""\n" >> ./../../Include/TmxTl1MarketDataFeedClient/Version.hpp

#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/TmxIpMarketDataFeedClient
printf "#define TMX_IP_MARKET_DATA_FEED_CLIENT_VERSION \""> ./../../Include/TmxIpMarketDataFeedClient/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/TmxIpMarketDataFeedClient/Version.hpp
printf \""\n" >> ./../../Include/TmxIpMarketDataFeedClient/Version.hpp

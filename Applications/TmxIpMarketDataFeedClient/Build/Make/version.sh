#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/TmxIpMarketDataFeedClient
printf "#define TMX_IP_MARKET_DATA_FEED_CLIENT_VERSION \""> ./../../Include/TmxIpMarketDataFeedClient/Version.hpp
hg id -n | tr -d "\n" >> ./../../Include/TmxIpMarketDataFeedClient/Version.hpp
printf \" >> ./../../Include/TmxIpMarketDataFeedClient/Version.hpp

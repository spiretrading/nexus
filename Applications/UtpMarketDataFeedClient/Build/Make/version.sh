#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/UtpMarketDataFeedClient
printf "#define UTP_MARKET_DATA_FEED_CLIENT_VERSION \""> ./../../Include/UtpMarketDataFeedClient/Version.hpp
hg id -n | tr -d "\n" >> ./../../Include/UtpMarketDataFeedClient/Version.hpp
printf \" >> ./../../Include/UtpMarketDataFeedClient/Version.hpp

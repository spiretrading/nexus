#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/UtpMarketDataFeedClient
printf "#define UTP_MARKET_DATA_FEED_CLIENT_VERSION \""> ./../../Include/UtpMarketDataFeedClient/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/UtpMarketDataFeedClient/Version.hpp
printf \""\n" >> ./../../Include/UtpMarketDataFeedClient/Version.hpp

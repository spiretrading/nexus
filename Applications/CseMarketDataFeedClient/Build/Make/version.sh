#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/CseMarketDataFeedClient
printf "#define CSE_MARKET_DATA_FEED_CLIENT_VERSION \""> ./../../Include/CseMarketDataFeedClient/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/CseMarketDataFeedClient/Version.hpp
printf \""\n" >> ./../../Include/CseMarketDataFeedClient/Version.hpp

#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/AsxItchMarketDataFeedClient
printf "#define ASX_ITCH_MARKET_DATA_FEED_CLIENT_VERSION \""> ./../../Include/AsxItchMarketDataFeedClient/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/AsxItchMarketDataFeedClient/Version.hpp
printf \""\n" >> ./../../Include/AsxItchMarketDataFeedClient/Version.hpp

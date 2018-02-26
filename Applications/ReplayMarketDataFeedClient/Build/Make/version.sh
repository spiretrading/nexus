#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/ReplayMarketDataFeedClient
printf "#define REPLAY_MARKET_DATA_FEED_CLIENT_VERSION \""> ./../../Include/ReplayMarketDataFeedClient/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/ReplayMarketDataFeedClient/Version.hpp
printf \""\n" >> ./../../Include/ReplayMarketDataFeedClient/Version.hpp

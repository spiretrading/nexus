#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/CtaMarketDataFeedClient
printf "#define CTA_MARKET_DATA_FEED_CLIENT_VERSION \""> ./../../Include/CtaMarketDataFeedClient/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/CtaMarketDataFeedClient/Version.hpp
printf \""\n" >> ./../../Include/CtaMarketDataFeedClient/Version.hpp

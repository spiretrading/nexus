#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/MarketDataClientTemplate
printf "#define MARKET_DATA_CLIENT_TEMPLATE_VERSION \""> ./../../Include/MarketDataClientTemplate/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/MarketDataClientTemplate/Version.hpp
printf \""\n" >> ./../../Include/MarketDataClientTemplate/Version.hpp

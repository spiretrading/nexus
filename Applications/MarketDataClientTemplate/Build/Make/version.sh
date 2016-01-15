#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/MarketDataClientTemplate
printf "#define MARKET_DATA_CLIENT_TEMPLATE_VERSION \""> ./../../Include/MarketDataClientTemplate/Version.hpp
hg id -n | tr -d "\n" >> ./../../Include/MarketDataClientTemplate/Version.hpp
printf \" >> ./../../Include/MarketDataClientTemplate/Version.hpp

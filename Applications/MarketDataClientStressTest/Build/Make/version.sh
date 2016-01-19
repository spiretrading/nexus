#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/MarketDataClientStressTest
printf "#define MARKET_DATA_CLIENT_STRESS_TEST_VERSION \""> ./../../Include/MarketDataClientStressTest/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/MarketDataClientStressTest/Version.hpp
printf \""\n" >> ./../../Include/MarketDataClientStressTest/Version.hpp

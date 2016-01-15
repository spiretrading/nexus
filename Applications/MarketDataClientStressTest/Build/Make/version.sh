#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/MarketDataClientStressTest
printf "#define MARKET_DATA_CLIENT_STRESS_TEST_VERSION \""> ./../../Include/MarketDataClientStressTest/Version.hpp
hg id -n | tr -d "\n" >> ./../../Include/MarketDataClientStressTest/Version.hpp
printf \" >> ./../../Include/MarketDataClientStressTest/Version.hpp

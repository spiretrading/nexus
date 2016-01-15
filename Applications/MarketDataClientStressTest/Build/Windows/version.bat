cd %~dp0../..
mkdir Include
cd Include
mkdir MarketDataClientStressTest
cd %~dp0
printf "#define MARKET_DATA_CLIENT_STRESS_TEST_VERSION """> %~dp0../../Include/MarketDataClientStressTest/Version.hpp
hg id -n | tr -d "\n\" >> %~dp0../../Include/MarketDataClientStressTest/Version.hpp
printf """" >> %~dp0../../Include/MarketDataClientStressTest/Version.hpp

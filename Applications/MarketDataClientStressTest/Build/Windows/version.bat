cd %~dp0../..
mkdir Include
cd Include
mkdir MarketDataClientStressTest
cd %~dp0
printf "#define MARKET_DATA_CLIENT_STRESS_TEST_VERSION """> %~dp0../../Include/MarketDataClientStressTest/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/MarketDataClientStressTest/Version.hpp
printf """\n" >> %~dp0../../Include/MarketDataClientStressTest/Version.hpp

pushd %~dp0..\..
mkdir Include
pushd Include
mkdir MarketDataClientStressTest
popd
popd
printf "#define MARKET_DATA_CLIENT_STRESS_TEST_VERSION """> %~dp0../../Include/MarketDataClientStressTest/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/MarketDataClientStressTest/Version.hpp
printf """\n" >> %~dp0../../Include/MarketDataClientStressTest/Version.hpp

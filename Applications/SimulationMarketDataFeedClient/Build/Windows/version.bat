pushd %~dp0..\..
mkdir Include
pushd Include
mkdir SimulationMarketDataFeedClient
popd
popd
printf "#define SIMULATION_MARKET_DATA_FEED_CLIENT_VERSION """> %~dp0../../Include/SimulationMarketDataFeedClient/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/SimulationMarketDataFeedClient/Version.hpp
printf """\n" >> %~dp0../../Include/SimulationMarketDataFeedClient/Version.hpp

cd %~dp0../..
mkdir Include
cd Include
mkdir SimulationMarketDataFeedClient
cd %~dp0
printf "#define SIMULATION_MARKET_DATA_FEED_CLIENT_VERSION """> %~dp0../../Include/SimulationMarketDataFeedClient/Version.hpp
hg id -n | tr -d "\n\" >> %~dp0../../Include/SimulationMarketDataFeedClient/Version.hpp
printf """" >> %~dp0../../Include/SimulationMarketDataFeedClient/Version.hpp

cd %~dp0../..
mkdir Include
cd Include
mkdir MarketDataRelayServer
cd %~dp0
printf "#define MARKET_DATA_RELAY_SERVER_VERSION """> %~dp0../../Include/MarketDataRelayServer/Version.hpp
hg id -n | tr -d "\n\" >> %~dp0../../Include/MarketDataRelayServer/Version.hpp
printf """" >> %~dp0../../Include/MarketDataRelayServer/Version.hpp

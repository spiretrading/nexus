cd %~dp0../..
mkdir Include
cd Include
mkdir MarketDataRelayServer
cd %~dp0
printf "#define MARKET_DATA_RELAY_SERVER_VERSION """> %~dp0../../Include/MarketDataRelayServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/MarketDataRelayServer/Version.hpp
printf """\n" >> %~dp0../../Include/MarketDataRelayServer/Version.hpp

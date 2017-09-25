pushd %~dp0..\..
mkdir Include
pushd Include
mkdir MarketDataRelayServer
popd
popd
printf "#define MARKET_DATA_RELAY_SERVER_VERSION """> %~dp0../../Include/MarketDataRelayServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/MarketDataRelayServer/Version.hpp
printf """\n" >> %~dp0../../Include/MarketDataRelayServer/Version.hpp

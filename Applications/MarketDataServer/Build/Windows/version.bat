pushd %~dp0..\..
mkdir Include
pushd Include
mkdir MarketDataServer
popd
popd
printf "#define MARKET_DATA_SERVER_VERSION """> %~dp0../../Include/MarketDataServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/MarketDataServer/Version.hpp
printf """\n" >> %~dp0../../Include/MarketDataServer/Version.hpp

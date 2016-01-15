cd %~dp0../..
mkdir Include
cd Include
mkdir MarketDataServer
cd %~dp0
printf "#define MARKET_DATA_SERVER_VERSION """> %~dp0../../Include/MarketDataServer/Version.hpp
hg id -n | tr -d "\n\" >> %~dp0../../Include/MarketDataServer/Version.hpp
printf """" >> %~dp0../../Include/MarketDataServer/Version.hpp

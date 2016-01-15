cd %~dp0../..
mkdir Include
cd Include
mkdir MarketDataClientTemplate
cd %~dp0
printf "#define MARKET_DATA_CLIENT_TEMPLATE_VERSION """> %~dp0../../Include/MarketDataClientTemplate/Version.hpp
hg id -n | tr -d "\n\" >> %~dp0../../Include/MarketDataClientTemplate/Version.hpp
printf """" >> %~dp0../../Include/MarketDataClientTemplate/Version.hpp

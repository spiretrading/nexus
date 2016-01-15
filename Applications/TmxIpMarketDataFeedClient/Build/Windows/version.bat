cd %~dp0../..
mkdir Include
cd Include
mkdir TmxIpMarketDataFeedClient
cd %~dp0
printf "#define TMX_IP_MARKET_DATA_FEED_CLIENT_VERSION """> %~dp0../../Include/TmxIpMarketDataFeedClient/Version.hpp
hg id -n | tr -d "\n\" >> %~dp0../../Include/TmxIpMarketDataFeedClient/Version.hpp
printf """" >> %~dp0../../Include/TmxIpMarketDataFeedClient/Version.hpp

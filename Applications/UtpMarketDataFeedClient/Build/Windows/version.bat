cd %~dp0../..
mkdir Include
cd Include
mkdir UtpMarketDataFeedClient
cd %~dp0
printf "#define UTP_MARKET_DATA_FEED_CLIENT_VERSION """> %~dp0../../Include/UtpMarketDataFeedClient/Version.hpp
hg id -n | tr -d "\n\" >> %~dp0../../Include/UtpMarketDataFeedClient/Version.hpp
printf """" >> %~dp0../../Include/UtpMarketDataFeedClient/Version.hpp

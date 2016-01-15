cd %~dp0../..
mkdir Include
cd Include
mkdir ChiaMarketDataFeedClient
cd %~dp0
printf "#define CHIA_MARKET_DATA_FEED_CLIENT_VERSION """> %~dp0../../Include/ChiaMarketDataFeedClient/Version.hpp
hg id -n | tr -d "\n\" >> %~dp0../../Include/ChiaMarketDataFeedClient/Version.hpp
printf """" >> %~dp0../../Include/ChiaMarketDataFeedClient/Version.hpp

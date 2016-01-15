cd %~dp0../..
mkdir Include
cd Include
mkdir AsxItchMarketDataFeedClient
cd %~dp0
printf "#define ASX_ITCH_MARKET_DATA_FEED_CLIENT_VERSION """> %~dp0../../Include/AsxItchMarketDataFeedClient/Version.hpp
hg id -n | tr -d "\n\" >> %~dp0../../Include/AsxItchMarketDataFeedClient/Version.hpp
printf """" >> %~dp0../../Include/AsxItchMarketDataFeedClient/Version.hpp

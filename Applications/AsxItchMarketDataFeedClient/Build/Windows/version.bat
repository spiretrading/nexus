cd %~dp0../..
mkdir Include
cd Include
mkdir AsxItchMarketDataFeedClient
cd %~dp0
printf "#define ASX_ITCH_MARKET_DATA_FEED_CLIENT_VERSION """> %~dp0../../Include/AsxItchMarketDataFeedClient/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/AsxItchMarketDataFeedClient/Version.hpp
printf """\n" >> %~dp0../../Include/AsxItchMarketDataFeedClient/Version.hpp

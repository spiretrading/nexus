cd %~dp0../..
mkdir Include
cd Include
mkdir ChiaMarketDataFeedClient
cd %~dp0
printf "#define CHIA_MARKET_DATA_FEED_CLIENT_VERSION """> %~dp0../../Include/ChiaMarketDataFeedClient/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/ChiaMarketDataFeedClient/Version.hpp
printf """\n" >> %~dp0../../Include/ChiaMarketDataFeedClient/Version.hpp

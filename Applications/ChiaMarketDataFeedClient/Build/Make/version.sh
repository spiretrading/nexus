mkdir -p ./../../Include/ChiaMarketDataFeedClient
printf "#define CHIA_MARKET_DATA_FEED_CLIENT_VERSION \""> ./../../Include/ChiaMarketDataFeedClient/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/ChiaMarketDataFeedClient/Version.hpp
printf \""\n" >> ./../../Include/ChiaMarketDataFeedClient/Version.hpp

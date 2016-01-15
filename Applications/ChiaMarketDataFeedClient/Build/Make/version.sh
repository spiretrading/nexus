mkdir -p ./../../Include/ChiaMarketDataFeedClient
printf "#define CHIA_MARKET_DATA_FEED_CLIENT_VERSION \""> ./../../Include/ChiaMarketDataFeedClient/Version.hpp
hg id -n | tr -d "\n" >> ./../../Include/ChiaMarketDataFeedClient/Version.hpp
printf \" >> ./../../Include/ChiaMarketDataFeedClient/Version.hpp

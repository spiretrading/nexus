pushd %~dp0..\..
mkdir Include
pushd Include
mkdir ChiaMarketDataFeedClient
popd
popd
printf "#define CHIA_MARKET_DATA_FEED_CLIENT_VERSION """> %~dp0../../Include/ChiaMarketDataFeedClient/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/ChiaMarketDataFeedClient/Version.hpp
printf """\n" >> %~dp0../../Include/ChiaMarketDataFeedClient/Version.hpp

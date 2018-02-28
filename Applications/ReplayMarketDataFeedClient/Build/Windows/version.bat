pushd %~dp0..\..
mkdir Include
pushd Include
mkdir ReplayMarketDataFeedClient
popd
popd
printf "#define REPLAY_MARKET_DATA_FEED_CLIENT_VERSION """> %~dp0../../Include/ReplayMarketDataFeedClient/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/ReplayMarketDataFeedClient/Version.hpp
printf """\n" >> %~dp0../../Include/ReplayMarketDataFeedClient/Version.hpp

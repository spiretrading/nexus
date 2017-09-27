pushd %~dp0..\..
mkdir Include
pushd Include
mkdir TmxTl1MarketDataFeedClient
popd
popd
printf "#define TMX_TL1_MARKET_DATA_FEED_CLIENT_VERSION """> %~dp0../../Include/TmxTl1MarketDataFeedClient/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/TmxTl1MarketDataFeedClient/Version.hpp
printf """\n" >> %~dp0../../Include/TmxTl1MarketDataFeedClient/Version.hpp

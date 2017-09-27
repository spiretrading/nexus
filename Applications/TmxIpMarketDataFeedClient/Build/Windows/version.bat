pushd %~dp0..\..
mkdir Include
pushd Include
mkdir TmxIpMarketDataFeedClient
popd
popd
printf "#define TMX_IP_MARKET_DATA_FEED_CLIENT_VERSION """> %~dp0../../Include/TmxIpMarketDataFeedClient/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/TmxIpMarketDataFeedClient/Version.hpp
printf """\n" >> %~dp0../../Include/TmxIpMarketDataFeedClient/Version.hpp

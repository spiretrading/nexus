pushd %~dp0..\..
mkdir Include
pushd Include
mkdir UtpMarketDataFeedClient
popd
popd
printf "#define UTP_MARKET_DATA_FEED_CLIENT_VERSION """> %~dp0../../Include/UtpMarketDataFeedClient/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/UtpMarketDataFeedClient/Version.hpp
printf """\n" >> %~dp0../../Include/UtpMarketDataFeedClient/Version.hpp

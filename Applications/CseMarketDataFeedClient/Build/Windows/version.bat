pushd %~dp0..\..
mkdir Include
pushd Include
mkdir CseMarketDataFeedClient
popd
popd
printf "#define CSE_MARKET_DATA_FEED_CLIENT_VERSION """> %~dp0../../Include/CseMarketDataFeedClient/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/CseMarketDataFeedClient/Version.hpp
printf """\n" >> %~dp0../../Include/CseMarketDataFeedClient/Version.hpp

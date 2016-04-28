cd %~dp0../..
mkdir Include
cd Include
mkdir CtaMarketDataFeedClient
cd %~dp0
printf "#define CTA_MARKET_DATA_FEED_CLIENT_VERSION """> %~dp0../../Include/CtaMarketDataFeedClient/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/CtaMarketDataFeedClient/Version.hpp
printf """\n" >> %~dp0../../Include/CtaMarketDataFeedClient/Version.hpp

pushd %~dp0..\..
mkdir Include
pushd Include
mkdir MarketDataClientTemplate
popd
popd
printf "#define MARKET_DATA_CLIENT_TEMPLATE_VERSION """> %~dp0../../Include/MarketDataClientTemplate/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/MarketDataClientTemplate/Version.hpp
printf """\n" >> %~dp0../../Include/MarketDataClientTemplate/Version.hpp

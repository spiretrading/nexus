pushd %~dp0..\..
mkdir Include
pushd Include
mkdir RiskServer
popd
popd
printf "#define RISK_SERVER_VERSION """> %~dp0../../Include/RiskServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/RiskServer/Version.hpp
printf """\n" >> %~dp0../../Include/RiskServer/Version.hpp

cd %~dp0../..
mkdir Include
cd Include
mkdir RiskServer
cd %~dp0
printf "#define RISK_SERVER_VERSION """> %~dp0../../Include/RiskServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/RiskServer/Version.hpp
printf """\n" >> %~dp0../../Include/RiskServer/Version.hpp

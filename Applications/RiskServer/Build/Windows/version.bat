cd %~dp0../..
mkdir Include
cd Include
mkdir RiskServer
cd %~dp0
printf "#define RISK_SERVER_VERSION """> %~dp0../../Include/RiskServer/Version.hpp
hg id -n | tr -d "\n\" >> %~dp0../../Include/RiskServer/Version.hpp
printf """" >> %~dp0../../Include/RiskServer/Version.hpp

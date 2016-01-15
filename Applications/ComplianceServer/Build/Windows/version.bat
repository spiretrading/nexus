cd %~dp0../..
mkdir Include
cd Include
mkdir ComplianceServer
cd %~dp0
printf "#define COMPLIANCE_SERVER_VERSION """> %~dp0../../Include/ComplianceServer/Version.hpp
hg id -n | tr -d "\n\" >> %~dp0../../Include/ComplianceServer/Version.hpp
printf """" >> %~dp0../../Include/ComplianceServer/Version.hpp

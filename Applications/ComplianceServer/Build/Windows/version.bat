cd %~dp0../..
mkdir Include
cd Include
mkdir ComplianceServer
cd %~dp0
printf "#define COMPLIANCE_SERVER_VERSION """> %~dp0../../Include/ComplianceServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/ComplianceServer/Version.hpp
printf """\n" >> %~dp0../../Include/ComplianceServer/Version.hpp

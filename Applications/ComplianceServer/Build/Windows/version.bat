pushd %~dp0..\..
mkdir Include
pushd Include
mkdir ComplianceServer
popd
popd
printf "#define COMPLIANCE_SERVER_VERSION """> %~dp0../../Include/ComplianceServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/ComplianceServer/Version.hpp
printf """\n" >> %~dp0../../Include/ComplianceServer/Version.hpp

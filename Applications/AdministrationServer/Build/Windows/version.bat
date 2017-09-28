pushd %~dp0..\..
mkdir Include
pushd Include
mkdir AdministrationServer
popd
popd
printf "#define ADMINISTRATION_SERVER_VERSION """> %~dp0../../Include/AdministrationServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/AdministrationServer/Version.hpp
printf """\n" >> %~dp0../../Include/AdministrationServer/Version.hpp

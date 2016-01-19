cd %~dp0../..
mkdir Include
cd Include
mkdir AdministrationServer
cd %~dp0
printf "#define ADMINISTRATION_SERVER_VERSION """> %~dp0../../Include/AdministrationServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/AdministrationServer/Version.hpp
printf """\n" >> %~dp0../../Include/AdministrationServer/Version.hpp

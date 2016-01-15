cd %~dp0../..
mkdir Include
cd Include
mkdir AdministrationServer
cd %~dp0
printf "#define ADMINISTRATION_SERVER_VERSION """> %~dp0../../Include/AdministrationServer/Version.hpp
hg id -n | tr -d "\n\" >> %~dp0../../Include/AdministrationServer/Version.hpp
printf """" >> %~dp0../../Include/AdministrationServer/Version.hpp

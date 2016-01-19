cd %~dp0../..
mkdir Include
cd Include
mkdir DefinitionsServer
cd %~dp0
printf "#define DEFINITIONS_SERVER_VERSION """> %~dp0../../Include/DefinitionsServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/DefinitionsServer/Version.hpp
printf """\n" >> %~dp0../../Include/DefinitionsServer/Version.hpp

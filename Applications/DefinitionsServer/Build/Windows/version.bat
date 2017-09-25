pushd %~dp0..\..
mkdir Include
pushd Include
mkdir DefinitionsServer
popd
popd
printf "#define DEFINITIONS_SERVER_VERSION """> %~dp0../../Include/DefinitionsServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/DefinitionsServer/Version.hpp
printf """\n" >> %~dp0../../Include/DefinitionsServer/Version.hpp

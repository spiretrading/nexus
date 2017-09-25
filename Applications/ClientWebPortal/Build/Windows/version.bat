pushd %~dp0..\..
mkdir Include
pushd Include
mkdir ClientWebPortal
popd
popd
printf "#define CLIENT_WEB_PORTAL_VERSION """> %~dp0../../Include/ClientWebPortal/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/ClientWebPortal/Version.hpp
printf """\n" >> %~dp0../../Include/ClientWebPortal/Version.hpp

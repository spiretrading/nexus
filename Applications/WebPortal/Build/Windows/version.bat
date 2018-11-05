pushd %~dp0..\..
mkdir Include
pushd Include
mkdir WebPortal
popd
popd
printf "#define WEB_PORTAL_VERSION """> %~dp0../../Include/WebPortal/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/WebPortal/Version.hpp
printf """\n" >> %~dp0../../Include/WebPortal/Version.hpp

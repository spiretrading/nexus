pushd %~dp0..\..
mkdir include
pushd include
mkdir web_portal
popd
popd
printf "#define WEB_PORTAL_VERSION """> %~dp0../../include/web_portal/version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../include/web_portal/version.hpp
printf """\n" >> %~dp0../../include/web_portal/version.hpp

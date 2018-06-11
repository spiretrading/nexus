pushd %~dp0..\..
mkdir include
pushd include
mkdir client_web_portal
popd
popd
printf "#define CLIENT_WEB_PORTAL_VERSION """> %~dp0../../include/client_web_portal/version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../include/client_web_portal/version.hpp
printf """\n" >> %~dp0../../include/client_web_portal/version.hpp

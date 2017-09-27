pushd %~dp0..\..
mkdir Include
pushd Include
mkdir ChartingServer
popd
popd
printf "#define CHARTING_SERVER_VERSION """> %~dp0../../Include/ChartingServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/ChartingServer/Version.hpp
printf """\n" >> %~dp0../../Include/ChartingServer/Version.hpp

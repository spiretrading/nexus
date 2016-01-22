cd %~dp0../..
mkdir Include
cd Include
mkdir ChartingServer
cd %~dp0
printf "#define CHARTING_SERVER_VERSION """> %~dp0../../Include/ChartingServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/ChartingServer/Version.hpp
printf """\n" >> %~dp0../../Include/ChartingServer/Version.hpp

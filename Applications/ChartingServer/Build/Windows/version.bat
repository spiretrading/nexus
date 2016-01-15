cd %~dp0../..
mkdir Include
cd Include
mkdir ChartingServer
cd %~dp0
printf "#define CHARTING_SERVER_VERSION """> %~dp0../../Include/ChartingServer/Version.hpp
hg id -n | tr -d "\n\" >> %~dp0../../Include/ChartingServer/Version.hpp
printf """" >> %~dp0../../Include/ChartingServer/Version.hpp

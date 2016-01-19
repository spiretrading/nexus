cd %~dp0../..
mkdir Include
cd Include
mkdir OasisOrderExecutionServer
cd %~dp0
printf "#define OASIS_ORDER_EXECUTION_SERVER_VERSION """> %~dp0../../Include/OasisOrderExecutionServer/Version.hpp
hg id -n | tr -d "\n\" >> %~dp0../../Include/OasisOrderExecutionServer/Version.hpp
printf """" >> %~dp0../../Include/OasisOrderExecutionServer/Version.hpp

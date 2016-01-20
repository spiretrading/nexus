cd %~dp0../..
mkdir Include
cd Include
mkdir OasisOrderExecutionServer
cd %~dp0
printf "#define OASIS_ORDER_EXECUTION_SERVER_VERSION """> %~dp0../../Include/OasisOrderExecutionServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/OasisOrderExecutionServer/Version.hpp
printf """\n" >> %~dp0../../Include/OasisOrderExecutionServer/Version.hpp

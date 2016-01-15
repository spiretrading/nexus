cd %~dp0../..
mkdir Include
cd Include
mkdir OrderExecutionRestore
cd %~dp0
printf "#define ORDER_EXECUTION_RESTORE_VERSION """> %~dp0../../Include/OrderExecutionRestore/Version.hpp
hg id -n | tr -d "\n\" >> %~dp0../../Include/OrderExecutionRestore/Version.hpp
printf """" >> %~dp0../../Include/OrderExecutionRestore/Version.hpp

cd %~dp0../..
mkdir Include
cd Include
mkdir OrderExecutionBackup
cd %~dp0
printf "#define ORDER_EXECUTION_BACKUP_VERSION """> %~dp0../../Include/OrderExecutionBackup/Version.hpp
hg id -n | tr -d "\n\" >> %~dp0../../Include/OrderExecutionBackup/Version.hpp
printf """" >> %~dp0../../Include/OrderExecutionBackup/Version.hpp

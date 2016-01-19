cd %~dp0../..
mkdir Include
cd Include
mkdir OrderExecutionBackup
cd %~dp0
printf "#define ORDER_EXECUTION_BACKUP_VERSION """> %~dp0../../Include/OrderExecutionBackup/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/OrderExecutionBackup/Version.hpp
printf """\n" >> %~dp0../../Include/OrderExecutionBackup/Version.hpp

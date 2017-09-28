pushd %~dp0..\..
mkdir Include
pushd Include
mkdir OrderExecutionBackup
popd
popd
printf "#define ORDER_EXECUTION_BACKUP_VERSION """> %~dp0../../Include/OrderExecutionBackup/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/OrderExecutionBackup/Version.hpp
printf """\n" >> %~dp0../../Include/OrderExecutionBackup/Version.hpp

pushd %~dp0..\..
mkdir Include
pushd Include
mkdir OrderExecutionRestore
popd
popd
printf "#define ORDER_EXECUTION_RESTORE_VERSION """> %~dp0../../Include/OrderExecutionRestore/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/OrderExecutionRestore/Version.hpp
printf """\n" >> %~dp0../../Include/OrderExecutionRestore/Version.hpp

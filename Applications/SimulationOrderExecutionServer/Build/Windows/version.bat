pushd %~dp0..\..
mkdir Include
pushd Include
mkdir SimulationOrderExecutionServer
popd
popd
printf "#define SIMULATION_ORDER_EXECUTION_SERVER_VERSION """> %~dp0../../Include/SimulationOrderExecutionServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/SimulationOrderExecutionServer/Version.hpp
printf """\n" >> %~dp0../../Include/SimulationOrderExecutionServer/Version.hpp

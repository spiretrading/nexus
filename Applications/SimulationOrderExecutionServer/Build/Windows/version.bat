cd %~dp0../..
mkdir Include
cd Include
mkdir SimulationOrderExecutionServer
cd %~dp0
printf "#define SIMULATION_ORDER_EXECUTION_SERVER_VERSION """> %~dp0../../Include/SimulationOrderExecutionServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/SimulationOrderExecutionServer/Version.hpp
printf """\n" >> %~dp0../../Include/SimulationOrderExecutionServer/Version.hpp

cd %~dp0../..
mkdir Include
cd Include
mkdir SimulationOrderExecutionServer
cd %~dp0
printf "#define SIMULATION_ORDER_EXECUTION_SERVER_VERSION """> %~dp0../../Include/SimulationOrderExecutionServer/Version.hpp
hg id -n | tr -d "\n\" >> %~dp0../../Include/SimulationOrderExecutionServer/Version.hpp
printf """" >> %~dp0../../Include/SimulationOrderExecutionServer/Version.hpp

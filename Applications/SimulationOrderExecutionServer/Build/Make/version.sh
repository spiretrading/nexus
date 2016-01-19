#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/SimulationOrderExecutionServer
printf "#define SIMULATION_ORDER_EXECUTION_SERVER_VERSION \""> ./../../Include/SimulationOrderExecutionServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/SimulationOrderExecutionServer/Version.hpp
printf \""\n" >> ./../../Include/SimulationOrderExecutionServer/Version.hpp

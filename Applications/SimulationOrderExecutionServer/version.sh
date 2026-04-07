#!/bin/bash
DIRECTORY="$(cd -P "$(dirname "${BASH_SOURCE[0]}")" >/dev/null && pwd -P)"
exec "$DIRECTORY/../../Nexus/version.sh" "SIMULATION_ORDER_EXECUTION_SERVER"

#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/OrderExecutionRestore
printf "#define ORDER_EXECUTION_RESTORE_VERSION \""> ./../../Include/OrderExecutionRestore/Version.hpp
hg id -n | tr -d "\n" >> ./../../Include/OrderExecutionRestore/Version.hpp
printf \" >> ./../../Include/OrderExecutionRestore/Version.hpp

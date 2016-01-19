#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/OrderExecutionRestore
printf "#define ORDER_EXECUTION_RESTORE_VERSION \""> ./../../Include/OrderExecutionRestore/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/OrderExecutionRestore/Version.hpp
printf \""\n" >> ./../../Include/OrderExecutionRestore/Version.hpp

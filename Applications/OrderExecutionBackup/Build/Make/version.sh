#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/OrderExecutionBackup
printf "#define ORDER_EXECUTION_BACKUP_VERSION \""> ./../../Include/OrderExecutionBackup/Version.hpp
hg id -n | tr -d "\n" >> ./../../Include/OrderExecutionBackup/Version.hpp
printf \" >> ./../../Include/OrderExecutionBackup/Version.hpp

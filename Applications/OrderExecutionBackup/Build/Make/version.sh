#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/OrderExecutionBackup
printf "#define ORDER_EXECUTION_BACKUP_VERSION \""> ./../../Include/OrderExecutionBackup/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/OrderExecutionBackup/Version.hpp
printf \""\n" >> ./../../Include/OrderExecutionBackup/Version.hpp

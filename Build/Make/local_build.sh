#!/bin/bash
set -o errexit
set -o pipefail
directory=$1
if [ "$2" = "" ]
then
  config="install"
else
  config=$2
fi

let cores="`grep -c "processor" < /proc/cpuinfo` / 2 + 1"
if [ $cores -gt 8 ]
then
  cores=8
fi

cmake --build $directory --target $config -- -j$cores

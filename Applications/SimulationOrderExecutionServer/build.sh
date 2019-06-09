#!/bin/bash
set -o errexit
set -o pipefail
if [ "$2" = "" ]
then
  config="install"
else
  config=$2
fi
let cores="`grep -c "processor" < /proc/cpuinfo` / 2 + 1"
let mem="`grep -oP "MemTotal: +\K([[:digit:]]+)(?=.*)" < /proc/meminfo` / 8388608"
let jobs="$(($cores<$mem?$cores:$mem))"
cmake --build . --target $config -- -j$jobs

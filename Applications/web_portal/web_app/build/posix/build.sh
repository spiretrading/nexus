#!/bin/bash
set -o errexit
set -o pipefail
directory=$(pwd)
arguments="$@"
if [ $# -eq 0 ] || [ "$1" != "Debug" ] || [ "$2" != "Debug" ];
  then export PROD_ENV=1;
fi
pushd $directory/../../tests/build/posix
./build.sh $arguments
popd
pushd $directory/../../application/build/posix
./build.sh $arguments
popd

#!/bin/bash
set -o errexit
set -o pipefail
directory=$(pwd)
if [ "$1" = "" ]
then
  build_type="Release"
else
  build_type=$1
fi
./version.sh
pushd $directory
. ./set_env.sh
export LIB_INSTALL_DIRECTORY=$directory/../../library
export TEST_INSTALL_DIRECTORY=$directory/../../tests
export INSTALL_DIRECTORY=$directory/../../application
export PROJECT_BUILD_TYPE=$build_type
cmake -G"Unix Makefiles" -DCMAKE_PREFIX_PATH=$QTDIR/lib/cmake $directory/../config
unset PROJECT_BUILD_TYPE
popd

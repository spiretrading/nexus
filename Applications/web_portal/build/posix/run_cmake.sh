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
export INSTALL_DIRECTORY=$directory/../../application
export NEXUS_BUILD_TYPE=$build_type
cmake -G"Unix Makefiles" $directory/../config
unset NEXUS_BUILD_TYPE
popd

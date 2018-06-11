#!/bin/bash
set -o errexit
set -o pipefail
directory=$(pwd)
arguments="$@"
pushd $directory/../../tests/build/posix
./build.sh $arguments
popd
pushd $directory/../../application/build/posix
./build.sh $arguments
popd

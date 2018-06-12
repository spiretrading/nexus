#!/bin/bash
set -o errexit
set -o pipefail
arguments="$@"
pushd tests/build/posix
./build.sh $arguments
popd
pushd application/build/posix
./build.sh $arguments
popd

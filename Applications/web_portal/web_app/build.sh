#!/bin/bash
set -o errexit
set -o pipefail
arguments="$@"
pushd library
./build.sh $arguments
popd
pushd tests
./build.sh $arguments
popd
pushd application
./build.sh $arguments
popd

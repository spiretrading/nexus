#!/bin/bash
set -o errexit
set -o pipefail
pushd library
./build.sh "$@"
popd
pushd tests
./build.sh "$@"
popd
pushd application
./build.sh "$@"
popd

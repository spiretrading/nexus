#!/bin/bash
set -o errexit
set -o pipefail
directory=$(pwd)
pushd $directory/../../library/build/posix
./setup.sh
popd
pushd $directory/../../tests/build/posix
./setup.sh
popd
pushd $directory/../../application/build/posix
./setup.sh
popd

#!/bin/bash
set -o errexit
set -o pipefail
arguments="$@"
pushd scratch
./build.sh $arguments
popd

#!/bin/bash
set -o errexit
set -o pipefail
arguments="$@"
pushd login_page_tester
./build.sh $arguments
popd

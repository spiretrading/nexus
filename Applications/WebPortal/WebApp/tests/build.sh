#!/bin/bash
set -o errexit
set -o pipefail
arguments="$@"
pushd account_page_tester
./build.sh $arguments
popd
pushd dashboard_page_tester
./build.sh $arguments
popd
pushd entitlements_page_tester
./build.sh $arguments
popd
pushd loading_page_tester
./build.sh $arguments
popd
pushd login_page_tester
./build.sh $arguments
popd
pushd profile_page_tester
./build.sh $arguments
popd
pushd risk_page_tester
./build.sh $arguments
popd
pushd scratch
./build.sh $arguments
popd

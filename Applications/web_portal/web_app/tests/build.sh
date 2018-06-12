#!/bin/bash
set -o errexit
set -o pipefail
directory=$(pwd)
arguments="$@"
test_build_function() {
  pushd $directory
  ./build.sh $arguments
  popd
}
export -f test_build_function
export directory
export arguments

tests=""
tests+="login_page_tester "
parallel -j2 --no-notice test_build_function ::: $tests

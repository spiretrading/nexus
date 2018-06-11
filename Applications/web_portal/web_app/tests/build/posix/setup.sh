#!/bin/bash
set -o errexit
set -o pipefail
directory=$(pwd)

test_setup_function() {
  pushd $directory/../../$1/build/posix
  ./setup.sh
  popd
}

export -f test_setup_function
export directory

tests=""
tests+="login_page_tester "
parallel -j2 --no-notice test_setup_function ::: $tests

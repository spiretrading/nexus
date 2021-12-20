#!/bin/bash
source="${BASH_SOURCE[0]}"
while [ -h "$source" ]; do
  dir="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd -P)"
  source="$(readlink "$source")"
  [[ $source != /* ]] && source="$dir/$source"
done
directory="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd -P)"
root=$(pwd -P)
if [ ! -f "build.sh" ]; then
  ln -s "$directory/build.sh" build.sh
fi
if [ ! -f "configure.sh" ]; then
  ln -s "$directory/configure.sh" configure.sh
fi

build_function() {
  location="${@: -1}"
  if [ ! -d "$location" ]; then
    mkdir -p "$location"
  fi
  pushd "$location"
  "$directory/$location/build.sh" -DD="$root/../library/Dependencies" "${@:1:$#-1}" 2>&1
  popd
}

export -f build_function
export directory
export root

build_function "$@" "account_directory_page_tester"
targets+=" account_page_tester"
targets+=" compliance_page_tester"
targets+=" create_account_page_tester"
targets+=" dashboard_page_tester"
targets+=" entitlements_page_tester"
targets+=" group_info_page_tester"
targets+=" group_page_tester"
targets+=" loading_page_tester"
targets+=" login_page_tester"
targets+=" mock"
targets+=" page_not_found_page_tester"
targets+=" profile_page_tester"
targets+=" risk_page_tester"
targets+=" scratch"

cores=`grep -c "processor" < /proc/cpuinfo`
jobs="$(($cores))"
parallel -j$jobs --no-notice build_function "$@" ::: $targets

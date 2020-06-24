#!/bin/bash
source="${BASH_SOURCE[0]}"
while [ -h "$source" ]; do
  dir="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd)"
  source="$(readlink "$source")"
  [[ $source != /* ]] && source="$dir/$source"
done
directory="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd)"
root=$(pwd)
if [ ! -f "build.sh" ]; then
  ln -s "$directory/build.sh" build.sh
fi
if [ ! -f "configure.sh" ]; then
  ln -s "$directory/configure.sh" configure.sh
fi
projects=""
projects+=" account_directory_page_tester"
projects+=" account_page_tester"
projects+=" compliance_page_tester"
projects+=" create_account_page_tester"
projects+=" dashboard_page_tester"
projects+=" entitlements_page_tester"
projects+=" group_info_page_tester"
projects+=" loading_page_tester"
projects+=" login_page_tester"
projects+=" page_not_found_tester"
projects+=" profile_page_tester"
projects+=" risk_page_tester"
projects+=" scratch"
for i in $projects; do
  if [ ! -d "$i" ]; then
    mkdir "$i"
  fi
  pushd "$i"
  "$directory/$i/configure.sh" -DD="$root/Dependencies" "$@"
  popd
done

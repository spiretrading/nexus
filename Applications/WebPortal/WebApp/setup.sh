#!/bin/bash
source="${BASH_SOURCE[0]}"
while [ -h "$source" ]; do
  dir="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd)"
  source="$(readlink "$source")"
  [[ $source != /* ]] && source="$dir/$source"
done
directory="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd)"
root=$(pwd)
"$directory/../../../WebApi/setup.sh"
dali_commit="781c8f9a037ebe44b1da725d0d93d168532bff97"
if [ ! -d "dali" ]; then
  git clone https://www.github.com/eidolonsystems/dali.git
fi
pushd dali
if ! git merge-base --is-ancestor "$dali_commit" HEAD; then
  git checkout master
  git pull
  git checkout "$dali_commit"
fi
if [ ! -d WebApi ]; then
  mkdir WebApi
  pushd WebApi
  "$directory/../../../WebApi/configure.sh" -DD="$root"
  popd
fi

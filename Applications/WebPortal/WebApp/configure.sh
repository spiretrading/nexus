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
if [ ! -d "library" ]; then
  mkdir "library"
fi
pushd library
./configure.sh -DD="$root/Dependencies" "$@"
popd
if [ ! -d "tests" ]; then
  mkdir "tests"
fi
pushd tests
./configure.sh -DD="$root/Dependencies" "$@"
popd
if [ ! -d "application" ]; then
  mkdir "application"
fi
pushd application
./configure.sh -DD="$root/Dependencies" "$@"
popd

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
for i in "$@"
do
case $i in
  -DD=*)
  dependencies="${i#*=}"
  shift
  ;;
esac
done
if [ "$dependencies" == "" ]; then
  dependencies="$root/Dependencies"
fi
if [ ! -d "$dependencies" ]; then
  mkdir -p "$dependencies"
fi
pushd "$dependencies"
"$directory"/setup.sh
popd
if [ "$dependencies" != "$root/Dependencies" ] && [ ! -d Dependencies ]; then
  ln -s "$dependencies" Dependencies
fi
if [ "$directory" != "$root" ]; then
  if [ -d source ]; then
    rm -rf source
  fi
  ln -s "$directory/source" source
  if [ -L "package.json" ]; then
    rm package.json
  fi
  ln -s "$directory/package.json" package.json
  if [ -L "tsconfig.json" ]; then
    rm tsconfig.json
  fi
  ln -s "$directory/tsconfig.json" tsconfig.json
fi

#!/bin/bash
set -o errexit
set -o pipefail
source="${BASH_SOURCE[0]}"
while [ -h "$source" ]; do
  dir="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd -P)"
  source="$(readlink "$source")"
  [[ $source != /* ]] && source="$dir/$source"
done
directory="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd -P)"
root=$(pwd -P)
for i in "$@"; do
  case $i in
    -DD=*)
      dependencies="${i#*=}"
      shift
      ;;
    *)
      config="$i"
      shift
      ;;
  esac
done
if [ "$config" = "" ]; then
  if [ -f "CMakeFiles/config.txt" ]; then
    config=$(cat CMakeFiles/config.txt)
  else
    config="Release"
  fi
fi
if [ "$config" = "clean" ]; then
  git clean -ffxd -e *Dependencies*
  if [ -f "Dependencies/cache_files/nexus.txt" ]; then
    rm "Dependencies/cache_files/nexus.txt"
  fi
elif [ "$config" = "reset" ]; then
  git clean -ffxd
  if [ -f "Dependencies/cache_files/nexus.txt" ]; then
    rm "Dependencies/cache_files/nexus.txt"
  fi
else
  cores="`grep -c "processor" < /proc/cpuinfo` / 2 + 1"
  mem="`grep -oP "MemTotal: +\K([[:digit:]]+)(?=.*)" < /proc/meminfo` / 8388608"
  jobs="$(($cores<$mem?$cores:$mem))"
  if [ "$dependencies" != "" ]; then
    "$directory/configure.sh" $config -DD="$dependencies"
  else
    "$directory/configure.sh" $config
  fi
  cmake --build "$root" --target install -- -j$jobs
fi

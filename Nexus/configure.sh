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
  config="Release"
fi
if [ "$dependencies" = "" ]; then
  dependencies="$root/Dependencies"
fi
if [ ! -d "$dependencies" ]; then
  mkdir -p "$dependencies"
fi
pushd "$dependencies"
"$directory"/setup.sh
popd
if [ "$dependencies" != "$root/Dependencies" ] && [ ! -d Dependencies ]; then
  rm -rf Dependencies
  ln -s "$dependencies" Dependencies
fi
if [ -d "$directory/Include" ]; then
  include_count=$(find $directory/Include -name "*.hpp" | wc -l)
  if [ -f "CMakeFiles/hpp_count.txt" ]; then
    hpp_count=$(cat "CMakeFiles/hpp_count.txt")
    if [ "$include_count" != "$hpp_count" ]; then
      run_cmake=1
    fi
  else
    run_cmake=1
  fi
  if [ "$run_cmake" = "1" ]; then
    if [ ! -d "CMakeFiles" ]; then
      mkdir CMakeFiles
    fi
    echo $include_count > "CMakeFiles/hpp_count.txt"
  fi
fi
if [ -d "$directory/Source" ]; then
  source_count=$(find $directory/Source -name "*.cpp" | wc -l)
  if [ -f "CMakeFiles/cpp_count.txt" ]; then
    cpp_count=$(cat "CMakeFiles/cpp_count.txt")
    if [ "$source_count" != "$cpp_count" ]; then
      run_cmake=1
    fi
  else
    run_cmake=1
  fi
  if [ "$run_cmake" = "1" ]; then
    if [ ! -d "CMakeFiles" ]; then
      mkdir CMakeFiles
    fi
    echo $source_count > "CMakeFiles/cpp_count.txt"
  fi
fi
if [ "$run_cmake" = "1" ]; then
  cmake -S "$directory" -DCMAKE_BUILD_TYPE=$config -DD=$dependencies
fi

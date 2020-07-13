#!/bin/bash
if [ "$(uname -s)" = "Darwin" ]; then
  STAT='stat -x -t "%Y%m%d%H%M%S"'
else
  STAT='stat'
fi
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
if [ ! -d "CMakeFiles" ]; then
  run_cmake=1
else
  if [ ! -f "CMakeFiles/timestamp.txt" ]; then
    run_cmake=1
  else
    ct="$(find $directory -type f -name CMakeLists.txt | xargs $STAT | grep Modify | awk '{print $2 $3}' | sort -r | head -1)"
    mt="$($STAT CMakeFiles/timestamp.txt | grep Modify | awk '{print $2 $3}')"
    if [ "$ct" \> "$mt" ]; then
      run_cmake=1
    fi
  fi
fi
if [ "$run_cmake" = "1" ]; then
  if [ ! -d "CMakeFiles" ]; then
    mkdir CMakeFiles
  fi
  echo "timestamp" > "CMakeFiles/timestamp.txt"
fi
if [ -f "CMakeFiles/config.txt" ]; then
  config_hash=$(cat "CMakeFiles/config.txt")
  if [ "$config_hash" != "$config" ]; then
    run_cmake=1
  fi
else
  run_cmake=1
fi
if [ "$run_cmake" = "1" ]; then
  if [ ! -d "CMakeFiles" ]; then
    mkdir CMakeFiles
  fi
  echo $config > "CMakeFiles/config.txt"
fi
if [ "$dependencies" != "$root/Dependencies" ] && [ ! -d Dependencies ]; then
  rm -rf Dependencies
  ln -s "$dependencies" Dependencies
fi
if [ -d "$directory/Include" ]; then
  include_hash=$(find $root $directory/Include -name "*.hpp" | grep "^/" | md5sum | cut -d" " -f1)
  if [ -f "CMakeFiles/hpp_hash.txt" ]; then
    hpp_hash=$(cat "CMakeFiles/hpp_hash.txt")
    if [ "$include_hash" != "$hpp_hash" ]; then
      run_cmake=1
    fi
  else
    run_cmake=1
  fi
  if [ "$run_cmake" = "1" ]; then
    if [ ! -d "CMakeFiles" ]; then
      mkdir CMakeFiles
    fi
    echo $include_hash > "CMakeFiles/hpp_hash.txt"
  fi
fi
if [ -d "$directory/Source" ]; then
  source_hash=$(find $root $directory/Source -name "*.cpp" | grep "^/" | md5sum | cut -d" " -f1)
  if [ -f "CMakeFiles/cpp_hash.txt" ]; then
    cpp_hash=$(cat "CMakeFiles/cpp_hash.txt")
    if [ "$source_hash" != "$cpp_hash" ]; then
      run_cmake=1
    fi
  else
    run_cmake=1
  fi
  if [ "$run_cmake" = "1" ]; then
    if [ ! -d "CMakeFiles" ]; then
      mkdir CMakeFiles
    fi
    echo $source_hash > "CMakeFiles/cpp_hash.txt"
  fi
fi
if [ "$run_cmake" = "1" ]; then
  cmake -S "$directory" -DCMAKE_BUILD_TYPE=$config -DD="$dependencies"
fi

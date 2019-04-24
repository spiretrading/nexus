#!/bin/bash
source="${BASH_SOURCE[0]}"
while [ -h "$source" ]; do
  dir="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd)"
  source="$(readlink "$source")"
  [[ $source != /* ]] && source="$dir/$source"
done
directory="$(cd -P "$(dirname "$source" )" >/dev/null 2>&1 && pwd)"
root="$(pwd)"
if [ ! -f "run_cmake.sh" ]; then
  printf "$directory/run_cmake.sh \"\$@\"" > run_cmake.sh
  chmod +x run_cmake.sh
fi
if [ ! -f "build.sh" ]; then
  printf "$directory/build.sh \"\$@\"" > build.sh
  chmod +x build.sh
fi
targets="Beam"
targets+=" Applications/AdminClient"
targets+=" Applications/ClientTemplate"
targets+=" Applications/DataStoreProfiler"
targets+=" Applications/HttpFileServer"
targets+=" Applications/QueryStressTest"
targets+=" Applications/RegistryServer"
targets+=" Applications/ServiceLocator"
targets+=" Applications/ServiceProtocolProfiler"
targets+=" Applications/ServletTemplate"
targets+=" Applications/UidServer"
targets+=" Applications/WebSocketEchoServer"

for i in $targets; do
  if [ ! -d "$i" ]; then
    mkdir -p "$i"
  fi
  pushd "$i"
  if [ ! -f "run_cmake.sh" ]; then
    printf "$directory/$i/run_cmake.sh \"\$@\"" > run_cmake.sh
    chmod +x run_cmake.sh
  fi
  $directory/$i/run_cmake.sh -DD="$root/Dependencies" "$@"
  popd
done

#!/bin/bash
if [ "$1" == "" ]; then
  config="Release"
else
  config="$1"
fi
python_directory=$(python3 -m site --user-site)
pushd ../Nexus/Dependencies/Beam/Applications
./install_python.sh "$@"
popd
mkdir -p "$python_directory"
cp "../Nexus/Libraries/$config/nexus.so" "$python_directory"

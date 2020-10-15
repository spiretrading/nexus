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
mkdir -p "$python_directory/nexus"
cp Python/__init__.py "$python_directory/nexus"
cp "../Nexus/Libraries/$config/_nexus.so" "$python_directory/nexus"

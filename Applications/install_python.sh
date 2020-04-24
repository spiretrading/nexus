#!/bin/bash
python_directory=$(python3 -m site --user-site)
cp ../Nexus/Dependencies/aspen/Libraries/Release/aspen.so $python_directory
mkdir -p $python_directory/beam
cp ../Nexus/Dependencies/Beam/Applications/Python/__init__.py \
  $python_directory/beam
cp ../Nexus/Dependencies/Beam/Beam/Libraries/Release/_beam.so \
  $python_directory/beam
mkdir -p $python_directory/nexus
cp Python/__init__.py $python_directory/nexus
cp ../Nexus/Libraries/Release/_nexus.so $python_directory/nexus

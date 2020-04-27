#!/bin/bash
python_directory=$(python3 -m site --user-site)
../Nexus/Dependencies/Applications/install_python.sh
mkdir -p $python_directory/nexus
cp Python/__init__.py $python_directory/nexus
cp ../Nexus/Libraries/Release/_nexus.so $python_directory/nexus

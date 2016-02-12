#!/bin/bash
directory=$(dirname $(readlink -f $0))
pushd $directory/../../Source/webapp/build/make
./setup.sh
popd

#!/bin/bash
directory=$(dirname $(readlink -f $0))
pushd $directory/
cd $directory/..
make $1
popd

#!/bin/bash
directory=$(dirname $(readlink -f $0))
pushd $directory/
cd $directory/../..
webpack
rm -rf ../../Application/webapp
cp -a dist/. ../../Application/webapp/
popd
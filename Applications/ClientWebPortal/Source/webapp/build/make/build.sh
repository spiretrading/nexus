#!/bin/bash
directory=$(dirname $(readlink -f $0))
pushd $directory/
cd $directory/..
npm install
# need to have webpack installed by executing
# npm install webpack -g
webpack
rm -rf ../../Application/webapp
cp -a dist/. ../../Application/webapp/
popd
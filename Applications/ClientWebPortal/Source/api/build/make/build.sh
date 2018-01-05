#!/bin/bash
config=$1
directory=$(dirname $(readlink -f $0))
pushd $directory/
pushd $directory/../..
if [ "$config" = "clean" ]
then
  rm -rf dist
elif [ "$config" = "Debug" ]
then
  npm run-script build-dev
else
  npm run-script build
fi
popd
popd

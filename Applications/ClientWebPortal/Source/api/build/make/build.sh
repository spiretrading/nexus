#!/bin/bash
config=$1
directory=$(dirname $(readlink -f $0))
pushd $directory/
cd $directory/../..
if [ "$config" = "clean" ]
then
  rm -rf dist
elif [ "$config" = "Debug" ]
then
  grunt build-dev
else
  grunt build-prod
fi
popd

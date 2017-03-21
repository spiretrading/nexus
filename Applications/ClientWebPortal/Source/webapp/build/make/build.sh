#!/bin/bash
config=$1
directory=$(dirname $(readlink -f $0))
pushd $directory/
cd $directory/../..
echo $config
if [ "$config" = "clean" ]
then
  rm -rf dist
else
  webpack --env.environment $config
  rm -rf ../../Application/webapp
  cp -a dist/. ../../Application/webapp/
fi
popd


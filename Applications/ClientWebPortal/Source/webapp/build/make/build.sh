#!/bin/bash
config=$1
directory=$(dirname $(readlink -f $0))
pushd $directory/
pushd $directory/../..
echo $config
if [ "$config" = "Clean" ]
then
  rm -rf dist
else
  sudo -u $(logname) rm -rf $directory/../../node_modules/spire-client
  sudo -u $(logname) ln -s $directory/../../../api/dist $directory/../../node_modules/spire-client
  if [ "$config" = "Debug" ]
  then
    npm run-script build-dev
  else
    npm run-script build
  fi
  rm -rf ../../Application/webapp
  cp -a dist/. ../../Application/webapp/
fi
popd
popd

#!/bin/bash
ENV=$1
directory=$(dirname $(readlink -f $0))
pushd $directory/
cd $directory/../..
if [ "$ENV" = "DEBUG" ]
then
  grunt build-dev
else
  grunt build-prod
fi
popd


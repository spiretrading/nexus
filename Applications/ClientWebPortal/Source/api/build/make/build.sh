#!/bin/bash
directory=$(dirname $(readlink -f $0))
pushd $directory/
cd $directory/../../..
rm -rf webapp/node_modules/spire-client
cp -r api webapp/node_modules/spire-client
popd

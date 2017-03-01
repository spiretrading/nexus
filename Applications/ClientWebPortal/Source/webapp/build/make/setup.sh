#!/bin/bash
directory=$(dirname $(readlink -f $0))
pushd $directory/
cd $directory/../../
{
    npm install
}
rm -rf $directory/../../node_modules/spire-client
ln -s $directory/../../../api/dist $directory/../../node_modules/spire-client
popd

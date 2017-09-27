#!/bin/bash
directory=$(dirname $(readlink -f $0))
pushd $directory/
pushd $directory/../../
{
  sudo -u $(logname) npm install
}
sudo -u $(logname) rm -rf $directory/../../node_modules/spire-client
sudo -u $(logname) ln -s $directory/../../../api/dist $directory/../../node_modules/spire-client
popd
popd

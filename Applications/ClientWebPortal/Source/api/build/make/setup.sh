#!/bin/bash
directory=$(dirname $(readlink -f $0))
pushd $directory/
cd $directory/../../
{
  sudo -u $(logname) npm install
}
popd

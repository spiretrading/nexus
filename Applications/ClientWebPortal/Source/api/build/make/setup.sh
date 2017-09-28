#!/bin/bash
directory=$(dirname $(readlink -f $0))
pushd $directory/
pushd $directory/../../
{
  sudo -u $(logname) npm install
}
popd
popd

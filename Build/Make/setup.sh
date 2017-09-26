#!/bin/bash
let cores="`grep -c "processor" < /proc/cpuinfo`"
directory=$(dirname $(readlink -f $0))

if [ ! -d "Beam" ]; then
  sudo -u $(logname) git clone https://www.github.com/eidolonsystems/beam.git Beam
fi
if [ -d "Beam" ]; then
  pushd Beam
  sudo -u $(logname) git checkout 030b1a36f7951d908fbcea19620d2a0fd8702b67
  popd
  ./Beam/Build/Make/setup.sh
  pushd ./Beam/Build/Make
  sudo -u $(logname) ./build.sh
  popd
fi

if [ ! -d "quickfix" ]; then
  sudo -u $(logname) wget http://prdownloads.sourceforge.net/quickfix/quickfix-1.14.3.tar.gz
  if [ -f quickfix-1.14.3.tar.gz ]; then
    sudo -u $(logname) gzip -d -c quickfix-1.14.3.tar.gz | sudo -u $(logname) tar -x
    pushd quickfix
    sudo -u $(logname) ./configure
    sudo -u $(logname) make -j $cores
    make install
    popd
    rm quickfix-1.14.3.tar.gz
  fi
fi

pushd $directory/../../Applications/ClientWebPortal/Build/Make
./setup.sh
popd

#!/bin/bash
let cores="`grep -c "processor" < /proc/cpuinfo`"
directory=$(dirname $(readlink -f $0))

if [ ! -d "Beam" ]; then
  sudo -u $(logname) git clone https://www.github.com/eidolonsystems/beam.git Beam
fi
if [ -d "Beam" ]; then
  pushd Beam
  sudo -u $(logname) git checkout master
  sudo -u $(logname) git pull
  sudo -u $(logname) git checkout eeddf4f4b49d8870233839583a47de8a5dbaf5fb
  popd
  ./Beam/Build/Make/setup.sh
  pushd ./Beam/Build/Make
  sudo -u $(logname) ./run_cmake.sh
  sudo -u $(logname) ./build.sh
  popd
fi

if [ ! -d "quickfix-v.1.14.4" ]; then
  sudo -u $(logname) wget https://github.com/quickfix/quickfix/archive/v.1.14.4.zip --no-check-certificate -O v.1.14.4.zip
  if [ -f v.1.14.4.zip ]; then
    sudo -u $(logname) unzip v.1.14.4.zip
    pushd quickfix-v.1.14.4
    sudo -u $(logname) ./bootstrap
    sudo -u $(logname) ./configure
    sudo -u $(logname) make -j $cores
    sudo -u $(logname) make check
    make install
    popd
    rm v.1.14.4.zip
  fi
fi

pushd $directory/../../Applications/ClientWebPortal/Build/Make
./setup.sh
popd

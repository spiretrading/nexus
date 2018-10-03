#!/bin/bash
let cores="`grep -c "processor" < /proc/cpuinfo`"
directory=$(dirname $(readlink -f $0))

beam_commit="7791540c510d3e581edb03b0fd910135da1e00bc"
if [ ! -d "Beam" ]; then
  sudo -u $(logname) git clone https://www.github.com/eidolonsystems/beam.git Beam
  pushd Beam
  sudo -u $(logname) git checkout "$beam_commit"
  popd
  ./Beam/Build/Make/setup.sh
  pushd ./Beam/Build/Make
  sudo -u $(logname) ./run_cmake.sh
  sudo -u $(logname) ./build.sh
  popd
fi
if [ -d "Beam" ]; then
  pushd Beam
  commit="`git log -1 | head -1 | awk '{ print $2 }'`"
  if [ "$commit" != "$beam_commit" ]; then
    sudo -u $(logname) git checkout master
    sudo -u $(logname) git pull
    sudo -u $(logname) git checkout "$beam_commit"
    popd
    ./Beam/Build/Make/setup.sh
    pushd ./Beam/Build/Make
    sudo -u $(logname) ./run_cmake.sh
    sudo -u $(logname) ./build.sh
  fi
  popd
fi

if [ ! -d "quickfix-v.1.15.1" ]; then
  sudo -u $(logname) wget https://github.com/quickfix/quickfix/archive/49b3508e48f0bbafbab13b68be72250bdd971ac2.zip -O quickfix-v.1.15.1.zip --no-check-certificate
  if [ -f quickfix-v.1.15.1.zip ]; then
    sudo -u $(logname) unzip quickfix-v.1.15.1.zip
    sudo -u $(logname) mv quickfix-49b3508e48f0bbafbab13b68be72250bdd971ac2 quickfix-v.1.15.1
    pushd ./quickfix-v.1.15.1
    pushd ./src/C++
    sudo -u $(logname) sed -i '105s/.*/template<typename T> using SmartPtr = std::shared_ptr<T>;/' Utility.h
    sudo -u $(logname) sed -i '108s/.*/template<typename T> using SmartPtr = std::shared_ptr<T>;/' Utility.h
    popd
    sudo -u $(logname) ./bootstrap
    sudo -u $(logname) ./configure
    sudo -u $(logname) make -j $cores
    make install
    popd
    rm quickfix-v.1.15.1.zip
  fi
fi

sudo -u $(logname) pip3 install argparse HTMLParser GitPython pymysql pytz pyyaml

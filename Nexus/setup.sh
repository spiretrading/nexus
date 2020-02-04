#!/bin/bash
let cores="`grep -c "processor" < /proc/cpuinfo`"
root="$(pwd)"
beam_commit="649d0901d49df4c8d7eb10626d9ce628edb20b5a"
build_beam=0
if [ ! -d "Beam" ]; then
  git clone https://www.github.com/eidolonsystems/beam.git Beam
  build_beam=1
fi
pushd Beam
if ! git merge-base --is-ancestor "$beam_commit" HEAD; then
  git checkout master
  git pull
  git checkout "$beam_commit"
  build_beam=1
fi
if [ "$build_beam" == "1" ]; then
  ./configure.sh "-DD=$root" Debug
  ./build.sh
  ./configure.sh "-DD=$root" Release
  ./build.sh
else
  pushd "$root"
  ./Beam/Beam/setup.sh
  popd
fi
popd
if [ ! -d "lua-5.3.5" ]; then
  wget http://www.lua.org/ftp/lua-5.3.5.tar.gz --no-check-certificate
  if [ -f lua-5.3.5.tar.gz ]; then
    gzip -d -c lua-5.3.5.tar.gz | tar -x
    pushd lua-5.3.5
    make -j $cores linux
    make local
    popd
    rm lua-5.3.5.tar.gz
  fi
fi
if [ ! -d "quickfix-v.1.15.1" ]; then
  wget https://github.com/quickfix/quickfix/archive/49b3508e48f0bbafbab13b68be72250bdd971ac2.zip -O quickfix-v.1.15.1.zip --no-check-certificate
  if [ -f quickfix-v.1.15.1.zip ]; then
    unzip quickfix-v.1.15.1.zip
    mv quickfix-49b3508e48f0bbafbab13b68be72250bdd971ac2 quickfix-v.1.15.1
    pushd quickfix-v.1.15.1
    pushd src/C++
    sed -i '105s/.*/template<typename T> using SmartPtr = std::shared_ptr<T>;/' Utility.h
    sed -i '108s/.*/template<typename T> using SmartPtr = std::shared_ptr<T>;/' Utility.h
    popd
    ./bootstrap
    ./configure --enable-shared=no --enable-static=yes 
    make -j $cores
    popd
    rm quickfix-v.1.15.1.zip
  fi
fi

pip3 install argparse
pip3 install HTMLParser
pip3 install GitPython

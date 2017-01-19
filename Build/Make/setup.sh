#!/bin/bash
let cores="`grep -c "processor" < /proc/cpuinfo`"
directory=$(dirname $(readlink -f $0))

if [ ! -d "quickfix" ]; then
  sudo -u $(logname) wget http://prdownloads.sourceforge.net/quickfix/quickfix-1.14.3.tar.gz
  if [ -f quickfix-1.14.3.tar.gz ]; then
    sudo -u $(logname) gzip -d -c quickfix-1.14.3.tar.gz | sudo -u $(logname) tar -x
    cd quickfix
    sudo -u $(logname) ./configure
    sudo -u $(logname) make -j $cores
    make install
    cd ..
    rm quickfix-1.14.3.tar.gz
  fi
fi

if [ ! -d "sqlite" ]; then
  sudo -u $(logname) wget http://www.sqlite.org/2015/sqlite-autoconf-3080803.tar.gz
  if [ -f sqlite-autoconf-3080803.tar.gz ]; then
    sudo -u $(logname) gzip -d -c sqlite-autoconf-3080803.tar.gz | sudo -u $(logname) tar -x
    sudo -u $(logname) mv sqlite-autoconf-3080803 sqlite
    cd sqlite
    sudo -u $(logname) ./configure
    sudo -u $(logname) make -j $cores
    make install
    cd ..
    rm sqlite-autoconf-3080803.tar.gz
  fi
fi

if [ ! -d "qt-5.5.0" ]; then
  sudo -u $(logname) wget http://download.qt.io/official_releases/qt/5.5/5.5.0/single/qt-everywhere-opensource-src-5.5.0.tar.xz
  if [ -f qt-everywhere-opensource-src-5.5.0.tar.xz ]; then
    sudo -u $(logname) tar xfv qt-everywhere-opensource-src-5.5.0.tar.xz
    rm qt-everywhere-opensource-src-5.5.0.tar.xz
    sudo -u $(logname) mv qt-everywhere-opensource-src-5.5.0 qt-5.5.0
    cd qt-5.5.0
    sudo -u $(logname) chmod +rwx configure
    sudo -u $(logname) ./configure -opensource -static -qt-xcb -make libs -make tools -opengl desktop -no-icu -qt-zlib -c++11 -confirm-license
    sudo -u $(logname) make -j $cores
    make install
  fi
fi

pushd $directory/../../Applications/ClientWebPortal/Build/Make
./setup.sh
popd

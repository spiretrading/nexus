#!/bin/bash
let cores="`grep -c "processor" < /proc/cpuinfo`"
directory=$(dirname $(readlink -f $0))

if [ ! -d "quickfix" ]; then
  wget http://prdownloads.sourceforge.net/quickfix/quickfix-1.14.3.tar.gz
  gzip -d -c quickfix-1.14.3.tar.gz | tar -x
  cd quickfix
  ./configure
  make -j $cores
  make install
  cd ..
  rm quickfix-1.14.3.tar.gz
fi

if [ ! -d "sqlite" ]; then
  wget https://www.sqlite.org/2015/sqlite-autoconf-3080803.tar.gz
  gzip -d -c sqlite-autoconf-3080803.tar.gz | tar -x
  mv sqlite-autoconf-3080803 sqlite
  cd sqlite
  ./configure
  make -j $cores
  make install
  cd ..
  rm sqlite-autoconf-3080803.tar.gz
fi

if [ ! -d "qt-5.5.0" ]; then
  wget http://download.qt.io/official_releases/qt/5.5/5.5.0/single/qt-everywhere-opensource-src-5.5.0.tar.xz
  tar xfv qt-everywhere-opensource-src-5.5.0.tar.xz
  rm qt-everywhere-opensource-src-5.5.0.tar.xz
  mv qt-everywhere-opensource-src-5.5.0 qt-5.5.0
  cd qt-5.5.0
  chmod +rwx configure
  ./configure -opensource -static -qt-xcb -make libs -make tools -opengl desktop -no-icu -qt-zlib -c++11 -confirm-license
  make -j $cores
  make install
fi

if [ ! -d "react-0.14.7" ]; then
  mkdir react-0.14.7
  cd react-0.14.7
  wget https://fb.me/react-0.14.7.js --no-check-certificate -O react-0.14.7.js
  wget https://fb.me/react-dom-0.14.7.js --no-check-certificate -O react-dom-0.14.7.js
  cd ..
fi

if [ ! -d "requirejs-2.1.22" ]; then
  mkdir requirejs-2.1.22
  cd requirejs-2.1.22
  wget http://requirejs.org/docs/release/2.1.22/minified/require.js
  cd ..
fi

pushd $directory/../../Applications/ClientWebPortal/Build/Make
./setup.sh
popd

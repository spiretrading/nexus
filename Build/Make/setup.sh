#!/bin/bash
let cores="`grep -c "processor" < /proc/cpuinfo`"
directory=$(dirname $(readlink -f $0))
username=$(echo ${SUDO_USER:-${USER}})

expected_commit="d181538e0a6b4635d8d8a6a111838f03550b7193"
if [ ! -d "Beam" ]; then
  sudo -u $username git clone https://www.github.com/eidolonsystems/beam.git Beam
  pushd Beam
  sudo -u $username git checkout "$expected_commit"
  popd
  ./Beam/Build/Make/setup.sh
  pushd ./Beam/Build/Make
  sudo -u $username ./run_cmake.sh
  sudo -u $username ./build.sh
  popd
fi
if [ -d "Beam" ]; then
  pushd Beam
  commit="`git log -1 | head -1 | awk '{ print $2 }'`"
  if [ "$commit" != "$expected_commit" ]; then
    sudo -u $username git checkout master
    sudo -u $username git pull
    sudo -u $username git checkout "$expected_commit"
    popd
    ./Beam/Build/Make/setup.sh
    pushd ./Beam/Build/Make
    sudo -u $username ./run_cmake.sh
    sudo -u $username ./build.sh
  fi
  popd
fi

if [ ! -d "qt-5.10.0" ]; then
  sudo -u $username git clone git://code.qt.io/qt/qt5.git qt-5.10.0
  if [ -d "qt-5.10.0" ]; then
    pushd qt-5.10.0
    sudo apt-get -y build-dep qt5-default
    apt-get install -y bison flex gperf ruby
    apt-get install -y libasound2-dev libcap-dev libdbus-1-dev \
      libdouble-conversion-dev libegl1-mesa-dev libfontconfig1-dev \
      libfreetype6-dev libglu1-mesa-dev libgstreamer0.10-dev \
      libgstreamer-plugins-base0.10-dev libice-dev libicu-dev \
      libnss3-dev libpci-dev libpulse-dev libsm-dev libssl-dev \
      libudev-dev libx11-dev libx11-xcb-dev '^libxcb.*-dev' \
      libxcomposite-dev libxcursor-dev libxdamage-dev libxext-dev \
      libxfixes-dev libxi-dev libxkbcommon-x11-dev libxrandr-dev \
      libxrender-dev libxslt-dev libxss-dev libxtst-dev
    sudo -u $username git checkout v5.10.0
    sudo -u $username perl init-repository --module-subset=default,-qtgamepad,-qtmultimedia,-qtspeech,-qtvirualkeyboard,-qtwebchannel,-qtwebengine,-qtwebglplugin,-qtwebkit,-qtwebkit-examples,-qtwebsockets,-qtwebview
    sudo -u $username ./configure -opensource -static -make libs -make tools \
      -nomake examples -nomake tests -gif -no-icu -no-opengl -qt-freetype \
      -qt-harfbuzz -qt-libpng -qt-pcre -qt-xcb -qt-zlib -confirm-license
    sudo -u $username make -j $cores
    make install
    popd
    ldconfig
  fi
fi

if [ ! -d "quickfix-v.1.14.4" ]; then
  sudo -u $username wget https://github.com/quickfix/quickfix/archive/v.1.14.4.zip --no-check-certificate -O v.1.14.4.zip
  if [ -f v.1.14.4.zip ]; then
    sudo -u $username unzip v.1.14.4.zip
    pushd quickfix-v.1.14.4
    sudo -u $username ./bootstrap
    sudo -u $username ./configure
    sudo -u $username make -j $cores
    sudo -u $username make check
    make install
    popd
    rm v.1.14.4.zip
  fi
fi

sudo -u $(logname) pip3 install argparse
sudo -u $(logname) pip3 install HTMLParser
sudo -u $(logname) pip3 install GitPython

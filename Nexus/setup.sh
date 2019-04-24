#!/bin/bash
let cores="`grep -c "processor" < /proc/cpuinfo`"
root="$(pwd)"
echo $root
if [ ! -d "cppunit-1.14.0" ]; then
  wget http://dev-www.libreoffice.org/src/cppunit-1.14.0.tar.gz --no-check-certificate
  if [ -f cppunit-1.14.0.tar.gz ]; then
    gzip -d -c cppunit-1.14.0.tar.gz | tar -x
    pushd cppunit-1.14.0
    touch configure.new
    cat configure | sed "s/\/\* automatically generated \*\//\$ac_prefix_conf_INP/" > configure.new
    mv configure.new configure
    chmod +x configure
    ./configure LDFLAGS='-ldl' --prefix="$root/cppunit-1.14.0"
    make -j $cores
    make install
    popd
    rm -f cppunit-1.14.0.tar.gz
  fi
fi
if [ ! -d "cryptopp565" ]; then
  wget https://github.com/weidai11/cryptopp/archive/b0f3b8ce1761e7ab9a3ead46fb7403fb38dd3723.zip -O cryptopp565.zip --no-check-certificate
  if [ -f cryptopp565.zip ]; then
    unzip cryptopp565.zip
    mv cryptopp-b0f3b8ce1761e7ab9a3ead46fb7403fb38dd3723 cryptopp565
    pushd cryptopp565
    make -j $cores
    make install PREFIX="$root/cryptopp565"
    popd
    rm -f cryptopp565.zip
  fi
fi
if [ ! -d "lua-5.3.1" ]; then
  wget http://www.lua.org/ftp/lua-5.3.1.tar.gz --no-check-certificate
  if [ -f lua-5.3.1.tar.gz ]; then
    gzip -d -c lua-5.3.1.tar.gz | tar -x
    pushd lua-5.3.1
    make -j $cores linux
    make local
    popd
    rm -f lua-5.3.1.tar.gz
  fi
fi
if [ ! -d "mysql-connector-c-6.1.11-src" ]; then
  wget https://dev.mysql.com/get/Downloads/Connector-C/mysql-connector-c-6.1.11-src.tar.gz --no-check-certificate
  if [ -f mysql-connector-c-6.1.11-src.tar.gz ]; then
    gzip -d -c mysql-connector-c-6.1.11-src.tar.gz | tar -x
    pushd mysql-connector-c-6.1.11-src
    cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX:PATH="$root/mysql-connector-c-6.1.11-src"
    make -j $cores
    make install
    popd
    rm -f mysql-connector-c-6.1.11-src.tar.gz
  fi
fi
if [ ! -d "mysql++-3.2.3" ]; then
  git clone https://github.com/eidolonsystems/mysqlpp mysql++-3.2.3
  if [ -d mysql++-3.2.3 ]; then
    pushd mysql++-3.2.3
    ./configure --enable-static --disable-shared --prefix="$root/mysql++-3.2.3" --with-mysql-include="$root/mysql-connector-c-6.1.11-src/include" --with-mysql-lib="$root/mysql-connector-c-6.1.11-src/lib"
    make -j $cores
    make install
    popd
  fi
fi
if [ ! -d "openssl-1.0.2g" ]; then
  wget https://www.openssl.org/source/openssl-1.0.2g.tar.gz --no-check-certificate
  if [ -f openssl-1.0.2g.tar.gz ]; then
    gzip -d -c openssl-1.0.2g.tar.gz | tar -x
    pushd openssl-1.0.2g
    export LDFLAGS=-ldl
    ./config no-shared threads -fPIC -ldl --prefix="$root/openssl-1.0.2g"
    make -j $cores
    make test
    make install
    unset LDFLAGS
    popd
    rm openssl-1.0.2g.tar.gz
  fi
fi
if [ ! -d "Python-3.7.2" ]; then
  wget https://www.python.org/ftp/python/3.7.2/Python-3.7.2.tgz --no-check-certificate
  if [ -f Python-3.7.2.tgz ]; then
    gzip -d -c Python-3.7.2.tgz | tar -xf -
    pushd Python-3.7.2
    export CFLAGS="-fPIC"
    ./configure --prefix="$root/Python-3.7.2"
    make -j $cores
    make install
    unset CFLAGS
    popd
    rm Python-3.7.2.tgz
  fi
fi
if [ ! -d "sqlite-amalgamation-3230100" ]; then
  wget https://www.sqlite.org/2018/sqlite-amalgamation-3230100.zip --no-check-certificate
  if [ -f sqlite-amalgamation-3230100.zip ]; then
    unzip sqlite-amalgamation-3230100.zip
    pushd sqlite-amalgamation-3230100
    gcc -c -O2 -o sqlite3.lib -DSQLITE_USE_URI=1 -fPIC sqlite3.c
    popd
    rm sqlite-amalgamation-3230100.zip
  fi
fi
if [ ! -d "tclap-1.2.1" ]; then
  wget "https://downloads.sourceforge.net/project/tclap/tclap-1.2.1.tar.gz?r=&ts=1309913922&use_mirror=superb-sea2" -O tclap-1.2.1.tar.gz --no-check-certificate
  if [ -f tclap-1.2.1.tar.gz ]; then
    gzip -d -c tclap-1.2.1.tar.gz | tar -x
    pushd tclap-1.2.1
    ./configure
    make -j $cores
    popd
    rm -f tclap-1.2.1.tar.gz
  fi
fi
if [ ! -d "viper" ]; then
  git clone https://www.github.com/eidolonsystems/viper
fi
if [ -d "viper" ]; then
  viper_commit="0631eff5a0a36d77bc45da1b0118dd49ea22953b"
  pushd viper
  commit="`git log -1 | head -1 | awk '{ print $2 }'`"
  if [ "$commit" != "$viper_commit" ]; then
    git checkout master
    git pull
    git checkout "$viper_commit"
  fi
  cmake -G "Unix Makefiles"
  popd
fi
if [ ! -d "yaml-cpp-0.6.2" ]; then
  git clone --branch yaml-cpp-0.6.2 https://github.com/jbeder/yaml-cpp.git yaml-cpp-0.6.2
  if [ -d "yaml-cpp-0.6.2" ]; then
    pushd yaml-cpp-0.6.2
    mkdir build
    popd
    pushd yaml-cpp-0.6.2/build
    export CFLAGS="-fPIC"
    export CXXFLAGS="-fPIC"
    cmake -DCMAKE_INSTALL_PREFIX:PATH="$root/yaml-cpp" ..
    make -j $cores
    unset CFLAGS
    unset CXXFLAGS
    popd
  fi
fi
if [ ! -d "zlib-1.2.11" ]; then
  wget https://github.com/madler/zlib/archive/v1.2.11.zip --no-check-certificate
  if [ -f v1.2.11.zip ]; then
    unzip v1.2.11.zip
    pushd zlib-1.2.11
    export CFLAGS="-fPIC"
    cmake -DCMAKE_INSTALL_PREFIX:PATH="$root/zlib-1.2.11" -G "Unix Makefiles"
    make -j $cores
    make install
    unset CFLAGS
    popd
    rm -f v1.2.11.zip
  fi
fi
if [ ! -d "boost_1_67_0" ]; then
  wget https://dl.bintray.com/boostorg/release/1.67.0/source/boost_1_67_0.tar.gz -O boost_1_67_0.tar.gz --no-check-certificate
  if [ -f boost_1_67_0.tar.gz ]; then
    tar xvf boost_1_67_0.tar.gz
    pushd boost_1_67_0
    pushd tools/build/src
    printf "using python : 3.7 : $root/Python-3.7.2 : $root/Python-3.7.2/include/python3.7m : $root/Python-3.7.2 ;\n" > user-config.jam
    popd
    export BOOST_BUILD_PATH=$(pwd)
    ./bootstrap.sh
    ./b2 -j$cores --prefix="$root/boost_1_67_0" cxxflags="-std=c++17 -fPIC" install
    popd
    unset BOOST_BUILD_PATH
    rm boost_1_67_0.tar.gz
  fi
fi

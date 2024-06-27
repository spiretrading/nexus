#!/bin/bash
exit_status=0
source="${BASH_SOURCE[0]}"
while [ -h "$source" ]; do
  dir="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd -P)"
  source="$(readlink "$source")"
  [[ $source != /* ]] && source="$dir/$source"
done
directory="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd -P)"
root="$(pwd -P)"
if [ "$(uname -s)" = "Darwin" ]; then
  STAT='stat -x -t "%Y%m%d%H%M%S"'
else
  STAT='stat'
fi
if [ -f "cache_files/nexus.txt" ]; then
  pt="$($STAT $directory/setup.sh | grep Modify | awk '{print $2 $3}')"
  mt="$($STAT cache_files/nexus.txt | grep Modify | awk '{print $2 $3}')"
  if [[ ! "$pt" > "$mt" ]]; then
    exit 0
  fi
fi
cores="`grep -c "processor" < /proc/cpuinfo`"
beam_commit="dc87a21f217230e2ae7c07651bbd141ac3a84897"
build_beam=0
if [ ! -d "Beam" ]; then
  git clone https://www.github.com/spiretrading/beam.git Beam
  if [ "$?" == "0" ]; then
    build_beam=1
    pushd Beam
    git checkout "$beam_commit"
    popd
  else
    rm -rf Beam
    exit_status=1
  fi
fi
if [ -d "Beam" ]; then
  pushd Beam
  if ! git merge-base --is-ancestor "$beam_commit" HEAD; then
    git checkout master
    git pull
    git checkout "$beam_commit"
    build_beam=1
  fi
  if [ "$build_beam" == "1" ]; then
    ./build.sh Debug -DD="$root"
    ./build.sh Release -DD="$root"
  else
    pushd "$root"
    ./Beam/Beam/setup.sh
    popd
  fi
  popd
fi
if [ ! -d "lua-5.4.4" ]; then
  wget http://www.lua.org/ftp/lua-5.4.4.tar.gz --no-check-certificate
  if [ "$?" == "0" ]; then
    gzip -d -c lua-5.4.4.tar.gz | tar -x
    pushd lua-5.4.4
    make -j $cores linux
    make local
    popd
  else
    exit_status=1
  fi
  rm -rf lua-5.4.4.tar.gz
fi
if [ ! -d "quickfix-v.1.15.1" ]; then
  wget https://github.com/quickfix/quickfix/archive/49b3508e48f0bbafbab13b68be72250bdd971ac2.zip -O quickfix-v.1.15.1.zip --no-check-certificate
  if [ "$?" == "0" ]; then
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
  else
    exit_status=1
  fi
  rm -rf quickfix-v.1.15.1.zip
fi
python3 -c "import git"
if [ "$?" != "0" ]; then
  pip3 install GitPython
fi
echo timestamp > cache_files/nexus.txt
exit $exit_status

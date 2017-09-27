#!/bin/bash
nexus_local_path=$(pwd)
if [ "$(uname -s)" = "Darwin" ]
then
  so_ext="dylib"
else
  so_ext="so"
fi
pushd /home/developers/Beam/Beam/Build/Make
. ./set_env.sh
popd
export PATH=/usr/local/Qt-5.5.0/bin:$PATH
export NEXUS_INCLUDE_PATH=$nexus_local_path/../../Include
export QUICK_FIX_INCLUDE_PATH=/home/developers/quickfix/include
export QUICK_FIX_LIBRARY_DEBUG_PATH=/home/developers/quickfix/lib/libquickfix.so
export QUICK_FIX_LIBRARY_OPTIMIZED_PATH=/home/developers/quickfix/lib/libquickfix.so

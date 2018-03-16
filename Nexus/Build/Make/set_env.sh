#!/bin/bash
nexus_local_path=$(pwd)
if [ "$(uname -s)" = "Darwin" ]
then
  so_ext="dylib"
else
  so_ext="so"
fi
pushd $nexus_local_path/../../../../Beam/Beam/Build/Make
. ./set_env.sh
popd
export PATH=/usr/local/Qt-5.5.0/bin:$PATH
export NEXUS_INCLUDE_PATH=$nexus_local_path/../../Include
export QUICK_FIX_INCLUDE_PATH=/usr/local/include
export QUICK_FIX_LIBRARY_DEBUG_PATH=/usr/local/lib/libquickfix.so
export QUICK_FIX_LIBRARY_OPTIMIZED_PATH=/usr/local/lib/libquickfix.so

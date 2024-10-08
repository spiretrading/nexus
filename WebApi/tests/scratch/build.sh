#!/bin/bash
if [ "$(uname -s)" = "Darwin" ]; then
  STAT='stat -x -t "%Y%m%d%H%M%S"'
else
  STAT='stat'
fi
BEAM_PATH=../../../../Beam/WebApi
NEXUS_PATH=../..
if [ $# -eq 0 ] || [ "$1" != "Debug" ]; then
  export PROD_ENV=1
fi
if [ ! -d "node_modules" ]; then
  UPDATE_NODE=1
else
  pushd node_modules
  if [ ! -f "mod_time.txt" ]; then
    UPDATE_NODE=1
  else
    pt="$($STAT ../package.json | grep Modify | awk '{print $2 $3}')"
    mt="$($STAT mod_time.txt | grep Modify | awk '{print $2 $3}')"
    if [ "$pt" \> "$mt" ]; then
      UPDATE_NODE=1
    fi
    if [ "$UPDATE_NODE" = "" ]; then
      if [ ! -d "../$BEAM_PATH/library" ]; then
        UPDATE_NODE=1
      else
        pt="$(find ../$BEAM_PATH/source -type f | xargs $STAT | grep Modify | awk '{print $2 $3}' | sort -r | head -1)"
        mt="$($STAT mod_time.txt | grep Modify | awk '{print $2 $3}')"
        if [ "$pt" \> "$mt" ]; then
          UPDATE_NODE=1
        fi
      fi
      if [ ! -d "../$NEXUS_PATH/library" ]; then
        UPDATE_NODE=1
      else
        pt="$(find ../$NEXUS_PATH/source -type f | xargs $STAT | grep Modify | awk '{print $2 $3}' | sort -r | head -1)"
        mt="$($STAT mod_time.txt | grep Modify | awk '{print $2 $3}')"
        if [ "$pt" \> "$mt" ]; then
          UPDATE_NODE=1
        fi
      fi
    fi
  fi
  popd
fi
if [ "$UPDATE_NODE" = "1" ]; then
  UPDATE_BUILD=1
  pushd $BEAM_PATH
  ./build.sh
  popd
  pushd $NEXUS_PATH
  ./build.sh
  popd
  npm install
  pushd node_modules
  cp -r ../$BEAM_PATH/library/* .
  if [ -d @types/beam ]; then
    rm -rf @types/beam
  fi
  mkdir -p @types/beam
  cp -r ../$BEAM_PATH/library/beam/* @types/beam
  cp -r ../$NEXUS_PATH/library/* .
  if [ -d @types/nexus ]; then
    rm -rf @types/nexus
  fi
  mkdir -p @types/nexus
  cp -r ../$NEXUS_PATH/library/nexus/* @types/nexus
  echo "timestamp" > mod_time.txt
  popd
fi
if [ ! -d "application" ]; then
  UPDATE_BUILD=1
else
  st="$(find source/ -type f | xargs $STAT | grep Modify | awk '{print $2 $3}' | sort -r | head -1)"
  lt="$(find application/ -type f | xargs $STAT | grep Modify | awk '{print $2 $3}' | sort -r | head -1)"
  if [ "$st" \> "$lt" ]; then
    UPDATE_BUILD=1
  fi
fi
if [ "$UPDATE_BUILD" = "1" ]; then
  if [ -d application ]; then
    rm -rf application
  fi
  npm run build
fi

#!/bin/bash
if [ "$(uname -s)" = "Darwin" ]; then
  STAT='stat -x -t "%Y%m%d%H%M%S"'
else
  STAT='stat'
fi
if [ "$1" = "clean" ]; then
  rm -rf ./library
  rm -rf ./node_modules/mod_time.txt
  exit 0
fi
if [ "$1" = "reset" ]; then
  rm -rf ./library
  rm -rf ./node_modules
  rm -rf ./package-lock.json
  exit 0
fi
BEAM_PATH=./../../Beam/WebApi
pushd $BEAM_PATH
./build.sh "$@"
popd
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
  fi
  popd
fi
if [ "$UPDATE_NODE" = "1" ]; then
  UPDATE_BUILD=1
  npm install
fi
if [ ! -d "library" ]; then
  UPDATE_BUILD=1
else
  st="$(find source/ -type f | xargs $STAT | grep Modify | awk '{print $2 $3}' | sort -r | head -1)"
  lt="$(find library/ -type f | xargs $STAT | grep Modify | awk '{print $2 $3}' | sort -r | head -1)"
  if [ "$st" \> "$lt" ]; then
    UPDATE_BUILD=1
  fi
fi
if [ ! -f "./node_modules/mod_time.txt" ]; then
  UPDATE_BUILD=1
else
  pt="$($STAT ./tsconfig.json | grep Modify | awk '{print $2 $3}')"
  dt="$($STAT $BEAM_PATH/node_modules/mod_time.txt | grep Modify | awk '{print $2 $3}')"
  mt="$($STAT ./node_modules/mod_time.txt | grep Modify | awk '{print $2 $3}')"
  if [ "$pt" \> "$mt" ]; then
    UPDATE_BUILD=1
  fi
  if [ "$dt" \> "$mt" ]; then
    UPDATE_BUILD=1
  fi
fi
if [ "$UPDATE_BUILD" = "1" ]; then
  if [ -d library ]; then
    rm -rf library
  fi
  npm run build
  echo "timestamp" > ./node_modules/mod_time.txt
fi

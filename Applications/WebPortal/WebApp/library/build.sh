#!/bin/bash
source="${BASH_SOURCE[0]}"
while [ -h "$source" ]; do
  dir="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd)"
  source="$(readlink "$source")"
  [[ $source != /* ]] && source="$dir/$source"
done
directory="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd)"
root=$(pwd)
for i in "$@"; do
  case $i in
    -DD=*)
      dependencies="${i#*=}"
      shift
      ;;
    *)
      config="$i"
      shift
      ;;
  esac
done
if [ "$(uname -s)" = "Darwin" ]; then
  STAT='stat -x -t "%Y%m%d%H%M%S"'
else
  STAT='stat'
fi
if [ "$config" = "clean" ]; then
  rm -rf library
  rm -f  mod_time.txt
  exit 0
fi
if [ "$config" = "reset" ]; then
  rm -rf library
  rm -f mod_time.txt
  rm -rf node_modules
  rm -f package-lock.json
  exit 0
fi
DALI_PATH=Dependencies/dali
NEXUS_PATH=Dependencies/WebApi
pushd $DALI_PATH
./build.sh "$@"
popd
pushd $NEXUS_PATH
./build.sh "$@"
popd
if [ ! -d "node_modules" ]; then
  UPDATE_NODE=1
else
  if [ ! -f "mod_time.txt" ]; then
    UPDATE_NODE=1
  else
    pt="$($STAT $directory/package.json | grep Modify | awk '{print $2 $3}')"
    mt="$($STAT mod_time.txt | grep Modify | awk '{print $2 $3}')"
    if [ "$pt" \> "$mt" ]; then
      UPDATE_NODE=1
    fi
  fi
fi
if [ "$UPDATE_NODE" = "1" ]; then
  UPDATE_BUILD=1
  if [ "$dependencies" != "" ]; then
    "$directory/configure.sh" -DD="$dependencies"
  else
    "$directory/configure.sh"
  fi
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
if [ ! -f "mod_time.txt" ]; then
  UPDATE_BUILD=1
else
  pt="$($STAT $directory/tsconfig.json | grep Modify | awk '{print $2 $3}')"
  dt="$($STAT $DALI_PATH/mod_time.txt | grep Modify | awk '{print $2 $3}')"
  nt="$($STAT $NEXUS_PATH/mod_time.txt | grep Modify | awk '{print $2 $3}')"
  mt="$($STAT mod_time.txt | grep Modify | awk '{print $2 $3}')"
  if [ "$pt" \> "$mt" ]; then
    UPDATE_BUILD=1
  fi
  if [ "$dt" \> "$mt" ]; then
    UPDATE_BUILD=1
  fi
  if [ "$nt" \> "$mt" ]; then
    UPDATE_BUILD=1
  fi
fi
if [ "$UPDATE_BUILD" = "1" ]; then
  if [ "$dependencies" != "" ]; then
    "$directory/configure.sh" -DD="$dependencies"
  else
    "$directory/configure.sh"
  fi
  if [ -d library ]; then
    rm -rf library
  fi
  npm run build
  echo "timestamp" > mod_time.txt
fi

#!/bin/bash
source="${BASH_SOURCE[0]}"
while [ -h "$source" ]; do
  dir="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd -P)"
  source="$(readlink "$source")"
  [[ $source != /* ]] && source="$dir/$source"
done
directory="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd -P)"
root=$(pwd -P)
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
if [ "$config" = "" ]; then
  config="Release"
fi
if [ "$(uname -s)" = "Darwin" ]; then
  STAT='stat -x -t "%Y%m%d%H%M%S"'
else
  STAT='stat'
fi
if [ "$config" = "clean" ]; then
  rm -rf application
  rm -f mod_time.txt
  exit 0
fi
if [ "$config" = "reset" ]; then
  rm -rf application
  rm -f mod_time.txt
  rm -rf node_modules
  rm -f package-lock.json
  exit 0
fi
if [ "$dependencies" != "" ]; then
  "$directory/configure.sh" -DD="$dependencies"
else
  "$directory/configure.sh"
fi
WEB_PORTAL_PATH=Dependencies/library
pushd $WEB_PORTAL_PATH
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
  npm install --no-package-lock
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
if [ ! -f "mod_time.txt" ]; then
  UPDATE_BUILD=1
else
  pt="$($STAT $directory/tsconfig.json | grep Modify | awk '{print $2 $3}')"
  wt="$($STAT $WEB_PORTAL_PATH/mod_time.txt | grep Modify | awk '{print $2 $3}')"
  mt="$($STAT mod_time.txt | grep Modify | awk '{print $2 $3}')"
  if [ "$pt" \> "$mt" ]; then
    UPDATE_BUILD=1
  fi
  if [ "$wt" \> "$mt" ]; then
    UPDATE_BUILD=1
  fi
fi
if [ "$UPDATE_BUILD" = "1" ]; then
  if [ -d application ]; then
    rm -rf application/*
  fi
  if [ "$config" = "Release" ]; then
    export PROD_ENV=1
  fi
  node node_modules/webpack/bin/webpack.js
  if [ "$config" = "Release" ]; then
    unset PROD_ENV
  fi
  echo "timestamp" > mod_time.txt
  if [ -d application ]; then
    cp -r "$directory/../resources" application
    cp -r source/index.html application
  fi
  if [ -d ../../Application ]; then
    mkdir -p ../../Application/web_app
    cp -r application/* ../../Application/web_app
  fi
fi

#!/bin/bash
BEAM_PATH=../../../../../../Beam/web_api
NEXUS_PATH=../../../../../web_api
WEB_PORTAL_PATH=../../library
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
    pt="$(ls -l --time-style=full-iso ../package.json | awk '{print $6 $7}')"
    mt="$(ls -l --time-style=full-iso mod_time.txt | awk '{print $6 $7}')"
    if [ "$pt" \> "$mt" ]; then
      UPDATE_NODE=1
    fi
    if [ "$UPDATE_NODE" = "" ]; then
      if [ ! -d "../$BEAM_PATH/library" ]; then
        UPDATE_NODE=1
      else
        pt="$(find ../$BEAM_PATH/source -type f | xargs ls -l --time-style=full-iso | awk '{print $6 $7}' | sort -r | head -1)"
        mt="$(ls -l --time-style=full-iso mod_time.txt | awk '{print $6 $7}')"
        if [ "$pt" \> "$mt" ]; then
          UPDATE_NODE=1
        fi
      fi
    fi
    if [ "$UPDATE_NODE" = "" ]; then
      if [ ! -d "../$NEXUS_PATH/library" ]; then
        UPDATE_NODE=1
      else
        pt="$(find ../$NEXUS_PATH/source -type f | xargs ls -l --time-style=full-iso | awk '{print $6 $7}' | sort -r | head -1)"
        mt="$(ls -l --time-style=full-iso mod_time.txt | awk '{print $6 $7}')"
        if [ "$pt" \> "$mt" ]; then
          UPDATE_NODE=1
        fi
      fi
    fi
    if [ "$UPDATE_NODE" = "" ]; then
      if [ ! -d "../$WEB_PORTAL_PATH/library" ]; then
        UPDATE_NODE=1
      else
        pt="$(find ../$WEB_PORTAL_PATH/source -type f | xargs ls -l --time-style=full-iso | awk '{print $6 $7}' | sort -r | head -1)"
        mt="$(ls -l --time-style=full-iso mod_time.txt | awk '{print $6 $7}')"
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
  pushd $WEB_PORTAL_PATH
  ./build.sh
  popd
  npm install
  pushd node_modules
  if [ -d beam ]; then
    rm -rf beam
  fi
  cp -r ../$BEAM_PATH/library/* .
  if [ -d @types/beam ]; then
    rm -rf @types/beam
  fi
  mkdir -p @types/beam
  cp -r ../$BEAM_PATH/library/beam/library/beam/* @types/beam
  if [ -d nexus ]; then
    rm -rf nexus
  fi
  cp -r ../$NEXUS_PATH/library/* .
  if [ -d @types/nexus ]; then
    rm -rf @types/nexus
  fi
  mkdir -p @types/nexus
  cp -r ../$NEXUS_PATH/library/nexus/library/nexus/* @types/nexus
  if [ -d web_portal ]; then
    rm -rf web_portal
  fi
  cp -r ../$WEB_PORTAL_PATH/library/* .
  if [ -d @types/web_portal ]; then
    rm -rf @types/web_portal
  fi
  mkdir -p @types/web_portal
  cp -r ../$WEB_PORTAL_PATH/library/web_portal/library/web_portal/* @types/web_portal
  echo "timestamp" > mod_time.txt
  popd
fi
if [ ! -d "application" ]; then
  UPDATE_BUILD=1
else
  st="$(find source/ -type f | xargs ls -l --time-style=full-iso | awk '{print $6 $7}' | sort -r | head -1)"
  lt="$(find application/ -type f | xargs ls -l --time-style=full-iso | awk '{print $6 $7}' | sort -r | head -1)"
  if [ "$st" \> "$lt" ]; then
    UPDATE_BUILD=1
  fi
fi
if [ "$UPDATE_BUILD" = "1" ]; then
  if [ -d application ]; then
    rm -rf application
  fi
  node ./node_modules/webpack/bin/webpack.js
  cp -r ../../resources application
  cp -r source/index.html application
fi

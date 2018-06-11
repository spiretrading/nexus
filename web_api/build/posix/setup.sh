#!/bin/bash
pushd ../../
npm install
pushd node_modules
if [ -d beam ]; then rm -rf beam; fi
cp -r ../../../Beam/web_api/library/* .
if [ -d @types/beam ]; then rm -rf @types/beam; fi
mkdir @types/beam
cp -r ../../../Beam/web_api/library/beam/library/beam/* @types/beam
popd

#!/bin/bash
pushd ../../
npm install
pushd node_modules
rm -rf beam
cp -r ../../../Beam/web_api/library/* .
rm -rf @types/beam
mkdir @types/beam
cp -r ../../../Beam/web_api/library/beam/library/beam/* @types/beam
popd

#!/bin/bash
pushd ../../
if [ $# -eq 0 ] || [ "$1" != "Debug" ]; then export PROD_ENV=1; fi
if [ -d library ]; then rm -rf library; fi
node ./node_modules/webpack/bin/webpack.js
popd

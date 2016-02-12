#!/bin/bash
directory=$(dirname $(readlink -f $0))
pushd $directory/
cd $directory/../..
nodejs $directory/../../node_modules/babel-cli/bin/babel.js *.jsx --out-dir ./output
popd
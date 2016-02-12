#!/bin/bash
directory=$(dirname $(readlink -f $0))
pushd $directory/
cd $directory/../../
npm install babel-cli babel-preset-es2015 babel-preset-react
popd

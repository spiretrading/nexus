#!/bin/bash
directory=$(dirname $(readlink -f $0))
pushd $directory/
cd $directory/../../
mkdir ../../Application/webapp/third-party
cp /home/developers/react-0.14.7/react-0.14.7.js ../../Application/webapp/third-party/react.js
cp /home/developers/react-0.14.7/react-dom-0.14.7.js ../../Application/webapp/third-party/react-dom.js
cp /home/developers/requirejs-2.1.22/* ../../Application/webapp/third-party
npm install babel-cli babel-preset-es2015 babel-preset-react
popd

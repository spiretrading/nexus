#!/bin/bash
directory=$(dirname $(readlink -f $0))
pushd $directory/
cd $directory/../../
mkdir ../../Application/webapp/third-party
cp /home/developers/jquery-1.12.1/jquery-1.12.1.min.js ../../Application/webapp/third-party/jquery.js
cp /home/developers/react-0.14.7/react-0.14.7.js ../../Application/webapp/third-party/react.js
cp /home/developers/react-0.14.7/react-dom-0.14.7.js ../../Application/webapp/third-party/react-dom.js
cp /home/developers/requirejs-2.1.22/* ../../Application/webapp/third-party
cp /home/developers/require-css/css.min.js ../../Application/webapp/third-party/css.js
npm install --save react-router
cp node_modules/react-router/umd/ReactRouter.min.js ../../Application/webapp/third-party/ReactRouter.js
npm install --save radium
cp node_modules/radium/dist/radium.min.js ../../Application/webapp/third-party/radium.js
npm install babel-cli babel-preset-es2015 babel-preset-react requirejs
popd

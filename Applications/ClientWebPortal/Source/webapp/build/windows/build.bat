SETLOCAL

pushd %~dp0
cd %~dp0../..
node %~dp0../../node_modules/babel-cli/bin/babel.js *.jsx --out-dir ./output
popd

ENDLOCAL

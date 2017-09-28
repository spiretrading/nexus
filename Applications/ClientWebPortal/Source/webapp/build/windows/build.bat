SETLOCAL
PUSHD %~dp0
PUSHD %~dp0../..
webpack
rm -rf ../../Application/webapp
cp -a dist/. ../../Application/webapp/
POPD
POPD
ENDLOCAL

SETLOCAL
PUSHD %~dp0
cd %~dp0../..
webpack
rm -rf ../../Application/webapp
cp -a dist/. ../../Application/webapp/
POPD
ENDLOCAL

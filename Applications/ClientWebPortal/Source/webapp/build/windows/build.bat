SETLOCAL

pushd %~dp0
cd %~dp0..
make
popd

ENDLOCAL

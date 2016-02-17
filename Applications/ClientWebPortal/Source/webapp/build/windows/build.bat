SETLOCAL

pushd %~dp0
cd %~dp0..
make %1
popd

ENDLOCAL

SETLOCAL
PUSHD %~dp0
PUSHD %~dp0..\..\
npm install
rm -rf %~dp0../../node_modules/spire-client
mklink /D "%~dp0../../../api" "%~dp0../../node_modules/spire-client"
POPD
POPD
ENDLOCAL

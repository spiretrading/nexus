SETLOCAL
PUSHD %~dp0..\..\
CALL npm install
PUSHD node_modules
rm -rf beam
cp -r ..\..\..\Beam\web_api\library\* .
rm -rf @types\beam
mkdir @types\beam
cp -r ..\..\..\Beam\web_api\library\beam\library\beam\* @types\beam
POPD
ENDLOCAL

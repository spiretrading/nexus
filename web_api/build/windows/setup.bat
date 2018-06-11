SETLOCAL
PUSHD %~dp0..\..\
CALL npm install
PUSHD ..\..\Beam\web_api\build\windows
CALL setup.bat
CALL build.bat
POPD
PUSHD node_modules
IF exist beam (
  rm -rf beam
)
cp -r ..\..\..\Beam\web_api\library\* .
IF exist @types\beam (
  rm -rf @types\beam
)
mkdir @types\beam
cp -r ..\..\..\Beam\web_api\library\beam\library\beam\* @types\beam
POPD
ENDLOCAL

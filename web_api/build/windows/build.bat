SETLOCAL
IF NOT "%1" == "Debug" (
  SET PROD_ENV=1
)
PUSHD %~dp0..\..\
IF exist library (
  rm -rf library
)
node .\node_modules\webpack\bin\webpack.js
POPD
ENDLOCAL

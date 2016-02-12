SETLOCAL

pushd %~dp0
cd %~dp0../../
npm install babel-cli babel-preset-es2015 babel-preset-react
popd

ENDLOCAL

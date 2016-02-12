SETLOCAL

pushd %~dp0
cd %~dp0../../Application/webapp/sources
npm install babel-cli babel-preset-es2015 babel-preset-react babel-preset-reacthmre
popd

ENDLOCAL

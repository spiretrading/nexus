SETLOCAL

pushd %~dp0
cd %~dp0../../
mkdir ..\..\Application\webapp\third-party
cp C:/Development/Libraries/react-0.14.7/react-0.14.7.js ../../Application/webapp/third-party/react.js
cp C:/Development/Libraries/react-0.14.7/react-dom-0.14.7.js ../../Application/webapp/third-party/react-dom.js
cp C:\Development\Libraries\requirejs-2.1.22/* ../../Application/webapp/third-party
npm install babel-cli babel-preset-es2015 babel-preset-react
popd

ENDLOCAL

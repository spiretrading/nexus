SETLOCAL

pushd %~dp0
cd %~dp0../../
mkdir ..\..\Application\webapp\third-party
cp C:/Development/Libraries/jquery-1.12.1/jquery-1.12.1.min.js ../../Application/webapp/third-party/jquery.js
cp C:/Development/Libraries/react-0.14.7/react-0.14.7.js ../../Application/webapp/third-party/react.js
cp C:/Development/Libraries/react-0.14.7/react-dom-0.14.7.js ../../Application/webapp/third-party/react-dom.js
cp C:/Development/Libraries/requirejs-2.1.22/* ../../Application/webapp/third-party
cp C:/Development/Libraries/require-css/css.min.js ../../Application/webapp/third-party/css.js
START /W CMD /C npm install --save react-router
cp node_modules/react-router/umd/ReactRouter.min.js ../../Application/webapp/third-party/ReactRouter.js
START /W CMD /C npm install babel-cli babel-preset-es2015 babel-preset-react requirejs
popd

ENDLOCAL

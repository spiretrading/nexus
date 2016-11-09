SETLOCAL
CALL %~dp0version.bat
CALL %~dp0set_env.bat
set INSTALL_DIRECTORY=%~dp0../../Application
set LIB_INSTALL_DIRECTORY=%~dp0../../Library
set TEST_INSTALL_DIRECTORY=%~dp0../../Tests
pushd %~dp0
cmake -DCMAKE_PREFIX_PATH=%QTDIR%/qtbase/lib/cmake %~dp0../Config
popd
ENDLOCAL

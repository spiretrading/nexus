SETLOCAL
CALL %~dp0version.bat
CALL %~dp0set_env.bat
set INSTALL_DIRECTORY=%~dp0../../application
set LIB_INSTALL_DIRECTORY=%~dp0../../library
set TEST_INSTALL_DIRECTORY=%~dp0../../tests
pushd %~dp0
cmake -T host=x64 -DCMAKE_PREFIX_PATH=%QTDIR%/qtbase/lib/cmake %~dp0../config
popd
ENDLOCAL

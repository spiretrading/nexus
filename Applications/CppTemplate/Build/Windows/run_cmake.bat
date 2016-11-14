SETLOCAL
CALL %~dp0version.bat
CALL %~dp0set_env.bat
set INSTALL_DIRECTORY=%~dp0../../Application
pushd %~dp0
cmake -DCMAKE_PREFIX_PATH=%QTDIR%/qtbase/lib/cmake %~dp0../Config
popd
ENDLOCAL

SETLOCAL
CALL %~dp0set_env.bat
set LIB_INSTALL_DIRECTORY=%~dp0../../Library
set TEST_INSTALL_DIRECTORY=%~dp0../../Tests
pushd %~dp0
cmake %~dp0../Config
popd
ENDLOCAL

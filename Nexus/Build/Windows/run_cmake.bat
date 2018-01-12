SETLOCAL
CALL %~dp0set_env.bat
set LIB_INSTALL_DIRECTORY=%~dp0../../Library
set TEST_INSTALL_DIRECTORY=%~dp0../../Tests
pushd %~dp0
cmake -G "Visual Studio 15 2017" -T host=x64 %~dp0../Config
popd
ENDLOCAL

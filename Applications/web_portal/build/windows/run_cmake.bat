SETLOCAL
CALL %~dp0version.bat
CALL %~dp0set_env.bat
set INSTALL_DIRECTORY=%~dp0../../application
pushd %~dp0
cmake -G "Visual Studio 15 2017" -T host=x64 %~dp0../config
popd
ENDLOCAL

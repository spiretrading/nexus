CALL %~dp0set_env.bat
set INSTALL_DIRECTORY=%~dp0../../Application
pushd %~dp0
cmake %~dp0../Config
popd

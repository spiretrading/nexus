SETLOCAL
IF [%1] == [] (
  SET GENERATOR=
) ELSE (
  SET GENERATOR=-G%1
)
CALL %~dp0version.bat
CALL %~dp0set_env.bat
set INSTALL_DIRECTORY=%~dp0../../Application
pushd %~dp0
cmake %~dp0../Config
popd
ENDLOCAL

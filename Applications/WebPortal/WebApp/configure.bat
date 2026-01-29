@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET ROOT=%cd%
SET DIRECTORY=%~dp0
SET EXIT_STATUS=0
IF NOT EXIST configure.bat (
  >configure.bat ECHO @ECHO OFF
  >>configure.bat ECHO CALL "%~dp0configure.bat" %%*
)
IF NOT EXIST build.bat (
  >build.bat ECHO @ECHO OFF
  >>build.bat ECHO CALL "%~dp0build.bat" %%*
)
CALL:configure library %*
CALL:configure tests %*
CALL:configure application %*
EXIT /B !EXIT_STATUS!
ENDLOCAL

:configure
SET PROJECT=%~1
IF NOT EXIST "!PROJECT!" (
  MD "!PROJECT!"
)
PUSHD "!PROJECT!"
CALL "!DIRECTORY!!PROJECT!\configure.bat" -DD="!ROOT!\library\Dependencies" %~2 %~3 %~4 %~5 %~6 %~7
IF ERRORLEVEL 1 SET EXIT_STATUS=1
POPD
EXIT /B 0

@ECHO OFF
SETLOCAL
SET ROOT=%cd%
IF NOT EXIST configure.bat (
  ECHO @ECHO OFF > configure.bat
  ECHO CALL "%~dp0configure.bat" %%* >> configure.bat
)
IF NOT EXIST build.bat (
  ECHO @ECHO OFF > build.bat
  ECHO CALL "%~dp0build.bat" %%* >> build.bat
)
CALL:configure library %*
CALL:configure tests %*
CALL:configure application %*
ENDLOCAL
EXIT /B %ERRORLEVEL%

:configure
IF NOT EXIST "%~1" (
  MD "%~1"
)
PUSHD "%~1"
CALL "%~dp0%~1\configure.bat" -DD="%ROOT%\library\Dependencies" %~2 %~3 %~4 %~5 %~6 %~7
POPD
EXIT /B 0

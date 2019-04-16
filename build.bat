@ECHO OFF
SETLOCAL
SET ROOT=%cd%
CALL:build Nexus %*
CALL:build Applications\AdministrationServer %*

ENDLOCAL
EXIT /B %ERRORLEVEL%

:build
IF NOT EXIST "%~1" (
  mkdir "%~1"
)
PUSHD "%~1"
CALL "%~dp0%~1\build.bat" %~2 %~3 %~4 %~5 %~6 %~7
POPD
EXIT /B 0

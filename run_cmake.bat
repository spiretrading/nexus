@ECHO OFF
SETLOCAL
SET ROOT=%cd%
IF NOT EXIST run_cmake.bat (
  echo CALL "%~dp0run_cmake.bat" %%* > run_cmake.bat
)
IF NOT EXIST build.bat (
  echo CALL "%~dp0build.bat" %%* > build.bat
)
CALL:run_cmake Nexus %*
CALL:run_cmake Applications\AdministrationServer %*
ENDLOCAL
EXIT /B %ERRORLEVEL%

:run_cmake
IF NOT EXIST "%~1" (
  mkdir "%~1"
)
PUSHD "%~1"
IF NOT EXIST run_cmake.bat (
  echo CALL "%~dp0%~1\run_cmake.bat" %%* > run_cmake.bat
)
CALL "%~dp0%~1\run_cmake.bat" -DD="%ROOT%\Dependencies" %~2 %~3 %~4 %~5 %~6 %~7
POPD
EXIT /B 0

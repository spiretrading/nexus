@ECHO OFF
SETLOCAL
IF NOT EXIST build.bat (
  echo CALL "%~dp0build.bat" %%* > build.bat
)
cmake -T host=x64 %* %~dp0
ENDLOCAL

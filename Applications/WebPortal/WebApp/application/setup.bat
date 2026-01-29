@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET "ROOT=%cd%"
SET "DIRECTORY=%~dp0"
CALL "!DIRECTORY!..\setup.bat" || EXIT /B 1
IF NOT EXIST library (
  MD library || EXIT /B 1
  PUSHD library
  CALL "!DIRECTORY!..\library\configure.bat" -DD="!ROOT!" || (
    POPD & EXIT /B 1
  )
  POPD
)
ENDLOCAL
EXIT /B 0

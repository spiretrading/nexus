@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET "DIRECTORY=%~dp0"
SET "ROOT=%cd%"
CALL :CreateForwardingScripts
CALL :ParseArgs %*
CALL :SetupDependencies || EXIT /B 1
CALL :CreateSymlinks || EXIT /B 1
ENDLOCAL
EXIT /B 0

:CreateForwardingScripts
IF NOT EXIST build.bat (
  ECHO @ECHO OFF > build.bat
  ECHO CALL "%~dp0build.bat" %%* >> build.bat
)
IF NOT EXIST configure.bat (
  ECHO @ECHO OFF > configure.bat
  ECHO CALL "%~dp0configure.bat" %%* >> configure.bat
)
EXIT /B 0

:ParseArgs
SET "DEPENDENCIES="
SET "IS_DEPENDENCY="
:ParseArgsLoop
SET "ARG=%~1"
IF "!ARG!"=="" (
  IF "!IS_DEPENDENCY!"=="1" (
    ECHO Error: -DD requires a path argument.
    EXIT /B 1
  )
  EXIT /B 0
)
IF "!IS_DEPENDENCY!"=="1" (
  SET "DEPENDENCIES=!ARG!"
  SET "IS_DEPENDENCY="
  SHIFT
  GOTO ParseArgsLoop
) ELSE (
  IF "!ARG:~0,4!"=="-DD=" (
    SET "DEPENDENCIES=!ARG:~4!"
    IF "!DEPENDENCIES!"=="" (
      ECHO Error: -DD requires a path argument.
      EXIT /B 1
    )
  ) ELSE IF "!ARG!"=="-DD" (
    SET "IS_DEPENDENCY=1"
  )
  SHIFT
  GOTO ParseArgsLoop
)
EXIT /B 0

:SetupDependencies
IF "!DEPENDENCIES!"=="" (
  SET "DEPENDENCIES=!ROOT!\Dependencies"
)
IF NOT EXIST "!DEPENDENCIES!" (
  MD "!DEPENDENCIES!" || EXIT /B 1
)
PUSHD "!DEPENDENCIES!" || EXIT /B 1
CALL "!DIRECTORY!setup.bat" || (POPD & EXIT /B 1)
POPD
EXIT /B 0

:CreateSymlinks
IF NOT "!DEPENDENCIES!"=="!ROOT!\Dependencies" (
  IF NOT EXIST Dependencies (
    mklink /j Dependencies "!DEPENDENCIES!" >NUL || EXIT /B 1
  )
)
IF NOT "!DIRECTORY!"=="!ROOT!\" (
  RD /S /Q source 2>NUL
  mklink /j source "!DIRECTORY!source" >NUL || EXIT /B 1
)
EXIT /B 0

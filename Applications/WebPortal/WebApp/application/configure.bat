@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET "ROOT=%cd%"
SET "DIRECTORY=%~dp0"
CALL :CreateForwardingScripts
CALL :ParseArgs %*
CALL :SetupDependencies || EXIT /B 1
CALL :CreateSymlinks || EXIT /B 1
ENDLOCAL
EXIT /B 0

:CreateForwardingScripts
IF NOT EXIST build.bat (
  >build.bat ECHO @ECHO OFF || EXIT /B 1
  >>build.bat ECHO CALL "%~dp0build.bat" %%* || EXIT /B 1
)
IF NOT EXIST configure.bat (
  >configure.bat ECHO @ECHO OFF || EXIT /B 1
  >>configure.bat ECHO CALL "%~dp0configure.bat" %%* || EXIT /B 1
)
EXIT /B 0

:ParseArgs
SET "DEPENDENCIES="
SET "IS_DEPENDENCY="
SET "IS_DIRECTORY="
:ParseArgsLoop
SET "ARG=%~1"
IF "!IS_DEPENDENCY!"=="1" (
  SET "DEPENDENCIES=!ARG!"
  SET "IS_DEPENDENCY="
  SHIFT
  GOTO ParseArgsLoop
) ELSE IF "!IS_DIRECTORY!"=="1" (
  SET "DIRECTORY=!ARG!"
  SET "IS_DIRECTORY="
  SHIFT
  GOTO ParseArgsLoop
) ELSE IF NOT "!ARG!"=="" (
  IF "!ARG:~0,4!"=="-DD=" (
    SET "DEPENDENCIES=!ARG:~4!"
  ) ELSE IF "!ARG!"=="-DD" (
    SET "IS_DEPENDENCY=1"
  ) ELSE IF "!ARG:~0,3!"=="-D=" (
    SET "DIRECTORY=!ARG:~3!"
  ) ELSE IF "!ARG!"=="-D" (
    SET "IS_DIRECTORY=1"
  )
  SHIFT
  GOTO ParseArgsLoop
)
IF "!DEPENDENCIES!"=="" (
  SET "DEPENDENCIES=!ROOT!\Dependencies"
)
EXIT /B 0

:SetupDependencies
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

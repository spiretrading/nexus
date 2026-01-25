@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET "ROOT=%cd%"
SET "DIRECTORY=%~dp0"
CALL :CreateForwardingScripts
CALL :ParseArgs %*
CALL :SetupDependencies || EXIT /B 1
CALL :CheckHashes || EXIT /B 1
CALL :RunCMake || EXIT /B 1
IF EXIST "!DIRECTORY!version.bat" CALL "!DIRECTORY!version.bat"
EXIT /B !ERRORLEVEL!
ENDLOCAL

:CreateForwardingScripts
IF NOT EXIST build.bat (
  >build.bat ECHO @ECHO OFF
  >>build.bat ECHO CALL "%~dp0build.bat" %%*
)
IF NOT EXIST configure.bat (
  >configure.bat ECHO @ECHO OFF
  >>configure.bat ECHO CALL "%~dp0configure.bat" %%*
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
PUSHD "!DEPENDENCIES!"
CALL "%~dp0setup.bat" || (POPD & EXIT /B 1)
POPD
IF NOT "!DEPENDENCIES!"=="!ROOT!\Dependencies" (
  IF EXIST Dependencies (
    RD /S /Q Dependencies || EXIT /B 1
  )
  mklink /j Dependencies "!DEPENDENCIES!" > NUL || EXIT /B 1
)
EXIT /B 0

:CheckHashes
SET "RUN_CMAKE="
IF NOT EXIST CMakeFiles (
  MD CMakeFiles || EXIT /B 1
  SET "RUN_CMAKE=1"
)
SET "TEMP_FILE=!ROOT!\temp_%RANDOM%%RANDOM%.txt"
TYPE "!DIRECTORY!CMakeLists.txt" > "!TEMP_FILE!"
IF EXIST "!DIRECTORY!Config" (
  FOR %%F IN ("!DIRECTORY!Config\*.cmake") DO TYPE "%%F" >> "!TEMP_FILE!"
  PUSHD "!DIRECTORY!Config"
  FOR /R %%F IN (*) DO (
    IF "%%~nxF"=="CMakeLists.txt" TYPE "%%F" >> "!TEMP_FILE!"
  )
  POPD
)
CALL :CheckFileHash "!TEMP_FILE!" "CMakeFiles\cmake_hash.txt"
IF EXIST "!DIRECTORY!Include" (
  DIR /a-d /b /s "!DIRECTORY!Include\*" > "!TEMP_FILE!"
  CALL :CheckFileHash "!TEMP_FILE!" "CMakeFiles\hpp_hash.txt"
)
IF EXIST "!DIRECTORY!Source" (
  DIR /a-d /b /s "!DIRECTORY!Source\*" > "!TEMP_FILE!"
  CALL :CheckFileHash "!TEMP_FILE!" "CMakeFiles\cpp_hash.txt"
)
EXIT /B 0

:CheckFileHash
SET CURRENT_HASH=
FOR /F "skip=1" %%H IN ('certutil -hashfile "%~1" SHA256') DO (
  IF NOT DEFINED CURRENT_HASH SET CURRENT_HASH=%%H
)
DEL "%~1"
IF EXIST "%~2" (
  SET /P CACHED_HASH=<"%~2"
  IF NOT "!CACHED_HASH!"=="!CURRENT_HASH!" SET RUN_CMAKE=1
) ELSE (
  SET RUN_CMAKE=1
)
IF "!RUN_CMAKE!"=="1" (
  >"%~2" ECHO !CURRENT_HASH!
)
SET CURRENT_HASH=
SET CACHED_HASH=
EXIT /B 0

:RunCMake
IF "!RUN_CMAKE!"=="1" (
  cmake -S "!DIRECTORY!." -DD="!DEPENDENCIES!" || EXIT /B 1
)
EXIT /B 0

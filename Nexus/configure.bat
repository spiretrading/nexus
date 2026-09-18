@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET "ROOT=%cd%"
SET "DIRECTORY=%~dp0"
CALL :ParseArgs %* || EXIT /B 1
CALL :CreateForwardingScripts || EXIT /B 1
CALL :SetupDependencies || EXIT /B 1
IF "!NEXUS_SKIP_CMAKE!"=="1" (
  CALL :RunVersion
  EXIT /B !ERRORLEVEL!
)
CALL :CheckHashes || EXIT /B 1
IF /I NOT "!DIRECTORY!"=="%~dp0" IF EXIST "!DIRECTORY!version.bat" (
  IF NOT EXIST Version.hpp SET "RUN_CMAKE=1"
)
IF "!RUN_CMAKE!"=="1" (
  CALL :GeneratedFiles begin || EXIT /B 1
)
CALL :ConfigureBuild
SET "CONFIGURE_ERROR=!ERRORLEVEL!"
IF "!RUN_CMAKE!"=="1" (
  CALL :GeneratedFiles end || EXIT /B 1
)
EXIT /B !CONFIGURE_ERROR!
ENDLOCAL

:ConfigureBuild
CALL :RunVersion || EXIT /B 1
CALL :RunCMake || EXIT /B 1
CALL :CommitHashes || EXIT /B 1
EXIT /B 0

:GeneratedFiles
cmake -DBUILD_DIRECTORY:PATH="!ROOT!" ^
  -DDEPENDENCIES_DIRECTORY:PATH="!DEPENDENCIES!" -DACTION=%1 ^
  -P "%~dp0Config\generated_files.cmake"
EXIT /B !ERRORLEVEL!

:CreateForwardingScripts
IF NOT EXIST build.bat (
  >build.bat ECHO @ECHO OFF
  >>build.bat ECHO CALL "!DIRECTORY!build.bat" %%*
  IF ERRORLEVEL 1 EXIT /B 1
)
IF NOT EXIST configure.bat (
  >configure.bat ECHO @ECHO OFF
  >>configure.bat ECHO CALL "!DIRECTORY!configure.bat" %%*
  IF ERRORLEVEL 1 EXIT /B 1
)
EXIT /B 0

:ParseArgs
SET "DEPENDENCIES="
SET "IS_DEPENDENCY="
SET "IS_DIRECTORY="
SET "CONFIG="
:ParseArgsLoop
SET "ARG=%~1"
IF "!ARG!"=="" (
  IF "!IS_DEPENDENCY!"=="1" (
    ECHO Error: -DD requires a path argument.
    EXIT /B 1
  )
  IF "!IS_DIRECTORY!"=="1" (
    ECHO Error: -D requires a path argument.
    EXIT /B 1
  )
  GOTO ParseArgsDone
)
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
) ELSE (
  IF "!ARG:~0,4!"=="-DD=" (
    SET "DEPENDENCIES=!ARG:~4!"
    IF "!DEPENDENCIES!"=="" (
      ECHO Error: -DD requires a path argument.
      EXIT /B 1
    )
  ) ELSE IF "!ARG!"=="-DD" (
    SET "IS_DEPENDENCY=1"
  ) ELSE IF "!ARG:~0,3!"=="-D=" (
    SET "DIRECTORY=!ARG:~3!"
    IF "!DIRECTORY!"=="" (
      ECHO Error: -D requires a path argument.
      EXIT /B 1
    )
  ) ELSE IF "!ARG!"=="-D" (
    SET "IS_DIRECTORY=1"
  ) ELSE (
    SET "CONFIG=!ARG!"
  )
  SHIFT
  GOTO ParseArgsLoop
)
:ParseArgsDone
IF "!CONFIG!"=="" (
  IF EXIST "CMakeFiles\config.txt" (
    SET /P CONFIG=<"CMakeFiles\config.txt"
  ) ELSE (
    SET "CONFIG=Release"
  )
)
IF /I "!CONFIG!"=="release" (
  SET "CONFIG=Release"
) ELSE IF /I "!CONFIG!"=="debug" (
  SET "CONFIG=Debug"
) ELSE IF /I "!CONFIG!"=="relwithdebinfo" (
  SET "CONFIG=RelWithDebInfo"
) ELSE IF /I "!CONFIG!"=="minsizerel" (
  SET "CONFIG=MinSizeRel"
) ELSE (
  ECHO Error: Invalid configuration "!CONFIG!".
  EXIT /B 1
)
IF "!DEPENDENCIES!"=="" (
  SET "DEPENDENCIES=!ROOT!\Dependencies"
)
FOR %%D IN ("!DIRECTORY!\.") DO (
  SET "DIRECTORY=%%~fD\"
)
EXIT /B 0

:SetupDependencies
FOR %%D IN ("!DEPENDENCIES!") DO (
  SET "DEPENDENCIES=%%~fD"
)
SET "DEPENDENCIES_ATTRIBUTES="
IF /I NOT "!DEPENDENCIES!"=="!ROOT!\Dependencies" (
  FOR %%D IN ("!ROOT!\Dependencies") DO (
    SET "DEPENDENCIES_ATTRIBUTES=%%~aD"
  )
  IF DEFINED DEPENDENCIES_ATTRIBUTES (
    IF "!DEPENDENCIES_ATTRIBUTES:l=!"=="!DEPENDENCIES_ATTRIBUTES!" (
      ECHO Error: !ROOT!\Dependencies exists and is not a link.
      EXIT /B 1
    )
  )
)
IF NOT EXIST "!DEPENDENCIES!" (
  MD "!DEPENDENCIES!" || EXIT /B 1
)
IF /I NOT "!NEXUS_SETUP_DIRECTORY!"=="!DEPENDENCIES!" (
  cmake -DDEPENDENCIES_DIRECTORY:PATH="!DEPENDENCIES!" ^
    -P "%~dp0Config\configure_dependencies.cmake" || EXIT /B 1
)
SET "NEXUS_SETUP_DIRECTORY=!DEPENDENCIES!"
IF /I NOT "!DEPENDENCIES!"=="!ROOT!\Dependencies" (
  IF DEFINED DEPENDENCIES_ATTRIBUTES (
    RD "!ROOT!\Dependencies" || EXIT /B 1
  )
  mklink /j "!ROOT!\Dependencies" "!DEPENDENCIES!" > NUL || EXIT /B 1
)
EXIT /B 0

:CheckHashes
SET "RUN_CMAKE="
SET "HASH_FILES="
IF /I "!DIRECTORY!"=="%~dp0" (
  IF NOT EXIST "CMakeFiles\clean_*.cmake" SET "RUN_CMAKE=1"
)
IF NOT EXIST CMakeCache.txt (
  SET "RUN_CMAKE=1"
) ELSE (
  SET "CACHED_CONFIG="
  SET "CONFIGURATION_TYPES="
  FOR /F "tokens=1,2,* delims=:=" %%K IN ('
      FINDSTR /B /C:"CMAKE_BUILD_TYPE:" /C:"CMAKE_CONFIGURATION_TYPES:" ^
        CMakeCache.txt') DO (
    IF "%%K"=="CMAKE_BUILD_TYPE" SET "CACHED_CONFIG=%%M"
    IF "%%K"=="CMAKE_CONFIGURATION_TYPES" SET "CONFIGURATION_TYPES=%%M"
  )
  IF NOT DEFINED CONFIGURATION_TYPES (
    IF NOT "!CACHED_CONFIG!"=="!CONFIG!" SET "RUN_CMAKE=1"
  )
)
IF NOT EXIST CMakeFiles (
  SET "RUN_CMAKE=1"
)
SET "TEMP_FILE=!ROOT!\temp_%RANDOM%%RANDOM%.txt"
>"!TEMP_FILE!" ECHO !CONFIG!
CALL :CheckFileHash "!TEMP_FILE!" "CMakeFiles\config_hash.txt"
>"!TEMP_FILE!" ECHO !DEPENDENCIES!
CALL :CheckFileHash "!TEMP_FILE!" "CMakeFiles\dependencies_hash.txt"
IF EXIST "!DIRECTORY!Include" (
  DIR /a-d /b /s "!DIRECTORY!Include\*" > "!TEMP_FILE!"
  CALL :CheckFileHash "!TEMP_FILE!" "CMakeFiles\hpp_hash.txt"
)
IF EXIST "!DIRECTORY!Source" (
  DIR /a-d /b /s "!DIRECTORY!Source\*" > "!TEMP_FILE!"
  CALL :CheckFileHash "!TEMP_FILE!" "CMakeFiles\cpp_hash.txt"
)
TYPE "!DIRECTORY!CMakeLists.txt" > "!TEMP_FILE!"
FOR %%F IN ("%~dp0Config\*.cmake") DO (
  TYPE "%%F" >> "!TEMP_FILE!"
)
IF EXIST "!DIRECTORY!Config" (
  FOR %%F IN ("!DIRECTORY!Config\*.cmake") DO (
    TYPE "%%F" >> "!TEMP_FILE!"
  )
  PUSHD "!DIRECTORY!Config" || EXIT /B 1
  FOR /R %%F IN (*) DO (
    IF "%%~nxF"=="CMakeLists.txt" TYPE "%%F" >> "!TEMP_FILE!"
  )
  POPD
)
CALL :CheckFileHash "!TEMP_FILE!" "CMakeFiles\cmake_hash.txt"
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
SET "HASH_FILES=!HASH_FILES! %~2"
SET "HASH[%~2]=!CURRENT_HASH!"
SET CURRENT_HASH=
SET CACHED_HASH=
EXIT /B 0

:RunCMake
IF "!RUN_CMAKE!"=="1" (
  DEL /F /Q "CMakeFiles\cmake_hash.txt" >NUL 2>&1
  IF EXIST "CMakeFiles\cmake_hash.txt" (
    ECHO Error: Unable to remove CMakeFiles\cmake_hash.txt.
    EXIT /B 1
  )
  SETLOCAL
  SET "NEXUS_SKIP_CMAKE=1"
  cmake -S "!DIRECTORY!." -DD="!DEPENDENCIES!" -DCMAKE_BUILD_TYPE=!CONFIG! || (
    ENDLOCAL
    EXIT /B 1
  )
  ENDLOCAL
)
EXIT /B 0

:CommitHashes
(ECHO !CONFIG!) >"CMakeFiles\config.txt" || EXIT /B 1
IF "!RUN_CMAKE!"=="1" (
  FOR %%F IN (!HASH_FILES!) DO (
    (ECHO !HASH[%%F]!) >"%%F" || EXIT /B 1
  )
)
EXIT /B 0

:RunVersion
IF EXIST "!DIRECTORY!version.bat" (
  FOR %%F IN ("!DIRECTORY!version.bat") DO (
    SET "DIR_VERSION=%%~fF"
  )
  FOR %%F IN ("%~dp0version.bat") DO (
    SET "SCRIPT_VERSION=%%~fF"
  )
  IF /I NOT "!DIR_VERSION!"=="!SCRIPT_VERSION!" (
    CALL "!DIRECTORY!version.bat" || EXIT /B 1
  )
)
EXIT /B 0

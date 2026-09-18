@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET "DIRECTORY=%~dp0"
SET "ROOT=%cd%"
CALL :ParseArgs %* || EXIT /B 1
IF /I "!CONFIG!"=="clean" (
  CALL :CleanBuild "clean"
  EXIT /B !ERRORLEVEL!
)
IF /I "!CONFIG!"=="reset" (
  CALL :CleanBuild "reset"
  EXIT /B !ERRORLEVEL!
)
CALL :Configure || EXIT /B 1
CALL :GeneratedFiles begin || EXIT /B 1
CALL :RunBuild
SET "BUILD_ERROR=!ERRORLEVEL!"
CALL :GeneratedFiles end || EXIT /B 1
EXIT /B !BUILD_ERROR!
ENDLOCAL

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
FOR %%D IN ("!DIRECTORY!\.") DO (
  SET "DIRECTORY=%%~fD\"
)
EXIT /B 0

:CleanBuild
SET "CLEAN_ERROR=0"
IF NOT EXIST "!ROOT!\CMakeCache.txt" (
  GOTO CleanConfiguration
)
CALL :GeneratedFiles begin || EXIT /B 1
cmake -DBUILD_DIRECTORY:PATH="!ROOT!" ^
  -P "%~dp0Config\native_clean.cmake" || SET "CLEAN_ERROR=1"
IF !CLEAN_ERROR! EQU 0 IF EXIST "!ROOT!\CMakeFiles\clean_outputs_*.cmake" (
  FOR %%F IN ("!ROOT!\CMakeFiles\clean_outputs_*.cmake") DO (
    cmake -P "%%F" || SET "CLEAN_ERROR=1"
  )
)
CALL :GeneratedFiles end || EXIT /B 1
:CleanConfiguration
IF !CLEAN_ERROR! EQU 0 (
  CALL :GeneratedFiles clean || SET "CLEAN_ERROR=1"
)
IF !CLEAN_ERROR! EQU 0 IF "%~1"=="reset" (
  cmake -DBUILD_DIRECTORY:PATH="!ROOT!" ^
    -P "%~dp0Config\reset.cmake" || SET "CLEAN_ERROR=1"
)
EXIT /B !CLEAN_ERROR!

:GeneratedFiles
cmake -DBUILD_DIRECTORY:PATH="!ROOT!" ^
  -DDEPENDENCIES_DIRECTORY:PATH="!DEPENDENCIES!" -DACTION=%1 ^
  -P "%~dp0Config\generated_files.cmake"
EXIT /B !ERRORLEVEL!

:Configure
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
IF NOT "!DEPENDENCIES!"=="" (
  CALL "!DIRECTORY!configure.bat" "!CONFIG!" -DD="!DEPENDENCIES!"
) ELSE (
  CALL "!DIRECTORY!configure.bat" "!CONFIG!"
)
EXIT /B !ERRORLEVEL!

:RunBuild
cmake --build "!ROOT!" --config "!CONFIG!" --parallel || EXIT /B 1
cmake --install "!ROOT!" --config "!CONFIG!" || EXIT /B 1
>"CMakeFiles\config.txt" ECHO !CONFIG!
EXIT /B 0

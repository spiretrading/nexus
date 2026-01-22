@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET DIRECTORY=%~dp0
SET ROOT=%cd%
:begin_args
SET ARG=%~1
IF "!IS_DEPENDENCY!" == "1" (
  SET DEPENDENCIES=!ARG!
  SET IS_DEPENDENCY=
  SHIFT
  GOTO begin_args
) ELSE IF NOT "!ARG!" == "" (
  IF "!ARG:~0,3!" == "-DD" (
    SET IS_DEPENDENCY=1
  ) ELSE (
    SET CONFIG=!ARG!
  )
  SHIFT
  GOTO begin_args
)
IF "!CONFIG!" == "clean" (
  git clean -ffxd -e *Dependencies*
  IF EXIST Dependencies\cache_files\nexus.txt (
    DEL Dependencies\cache_files\nexus.txt
  )
) ELSE IF "!CONFIG!" == "reset" (
  git clean -ffxd
  IF EXIST Dependencies\cache_files\nexus.txt (
    DEL Dependencies\cache_files\nexus.txt
  )
) ELSE (
  IF "!CONFIG!" == "" (
    IF EXIST CMakeFiles\config.txt (
      FOR /F %%i IN ('TYPE CMakeFiles\config.txt') DO (
        SET CONFIG=%%i
      )
    ) ELSE (
      SET CONFIG=Release
    )
  )
  IF /I "!CONFIG!"=="release" SET CONFIG=Release
  IF /I "!CONFIG!"=="debug" SET CONFIG=Debug
  IF /I "!CONFIG!"=="relwithdebinfo" SET CONFIG=RelWithDebInfo
  IF /I "!CONFIG!"=="minsizerel" SET CONFIG=MinSizeRel
  IF NOT "!DEPENDENCIES!" == "" (
    CALL "!DIRECTORY!configure.bat" -DD="!DEPENDENCIES!"
  ) ELSE (
    CALL "!DIRECTORY!configure.bat"
  )
  IF ERRORLEVEL 1 EXIT /B 1
  cmake --build "!ROOT!" --target INSTALL --config "!CONFIG!" --parallel
  IF ERRORLEVEL 1 EXIT /B 1
  >CMakeFiles\config.txt ECHO !CONFIG!
)
ENDLOCAL

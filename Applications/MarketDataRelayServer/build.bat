@ECHO OFF
SETLOCAL EnableDelayedExpansion
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
IF "!CONFIG!" == "" (
  SET CONFIG=Release
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
  IF NOT "!DEPENDENCIES!" == "" (
    CALL configure.bat -DD=!DEPENDENCIES!
  ) ELSE (
    CALL configure.bat
  )
  cmake --build "!ROOT!" --target INSTALL --config "!CONFIG!"
)
ENDLOCAL

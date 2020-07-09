@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET ROOT=%cd%
IF NOT EXIST build.bat (
  ECHO @ECHO OFF > build.bat
  ECHO CALL "%~dp0build.bat" %%* >> build.bat
)
IF NOT EXIST configure.bat (
  ECHO @ECHO OFF > configure.bat
  ECHO CALL "%~dp0configure.bat" %%* >> configure.bat
)
SET DIRECTORY=%~dp0
SET DEPENDENCIES=
SET IS_DEPENDENCY=
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
  )
  SHIFT
  GOTO begin_args
)
IF "!DEPENDENCIES!" == "" (
  SET DEPENDENCIES=!ROOT!\Dependencies
)
IF NOT EXIST "!DEPENDENCIES!" (
  MD "!DEPENDENCIES!"
)
PUSHD "!DEPENDENCIES!"
CALL "!DIRECTORY!setup.bat"
POPD
IF NOT "!DEPENDENCIES!" == "!ROOT!\Dependencies" (
  IF EXIST Dependencies (
    RD /S /Q Dependencies
  )
  mklink /j Dependencies "!DEPENDENCIES!" > NUL
)
SET RUN_CMAKE=
IF NOT EXIST CMakeFiles (
  SET RUN_CMAKE=1
) ELSE (
  IF NOT EXIST CMakeFiles\timestamp.txt (
    SET RUN_CMAKE=1
  ) ELSE (
    FOR /F %%i IN (
        'ls -l --time-style=full-iso !DIRECTORY!CMakeLists.txt !DIRECTORY!Config\* ^| grep CMakeLists.txt ^| awk "{print $6 $7}"') DO (
      FOR /F %%j IN (
          'ls -l --time-style=full-iso CMakeFiles\timestamp.txt ^| awk "{print $6 $7}"') DO (
        IF "%%i" GEQ "%%j" (
          SET RUN_CMAKE=1
        )
      )
    )
  )
)
IF "!RUN_CMAKE!" == "1" (
  IF NOT EXIST CMakeFiles (
    MD CMakeFiles
  )
  ECHO timestamp > CMakeFiles\timestamp.txt
)
IF EXIST "!DIRECTORY!Include" (
  DIR /a-d /b /s "!DIRECTORY!Include\*.hpp" > hpp_hash.txt
  SET C=0
  FOR /F %%i IN ('certutil -hashfile hpp_hash.txt') DO (
    IF !C!==1 (
      IF EXIST CMakeFiles\hpp_hash.txt (
        FOR /F %%j IN ('TYPE CMakeFiles\hpp_hash.txt') DO (
          IF NOT "%%i" == "%%j" (
            SET RUN_CMAKE=1
          )
        )
      ) ELSE (
        SET RUN_CMAKE=1
      )
      IF "!RUN_CMAKE!" == "1" (
        IF NOT EXIST CMakeFiles (
          MD CMakeFiles
        )
        ECHO %%i > CMakeFiles\hpp_hash.txt
      )
    )
    SET /A C=C+1
  )
  DEL hpp_hash.txt
)
IF EXIST "!DIRECTORY!Source" (
  DIR /a-d /b /s "!DIRECTORY!Source\*.cpp" > cpp_hash.txt
  SET C=0
  FOR /F %%i IN ('certutil -hashfile cpp_hash.txt') DO (
    IF !C!==1 (
      IF EXIST CMakeFiles\cpp_hash.txt (
        FOR /F %%j IN ('TYPE CMakeFiles\cpp_hash.txt') DO (
          IF NOT "%%i" == "%%j" (
            SET RUN_CMAKE=1
          )
        )
      ) ELSE (
        SET RUN_CMAKE=1
      )
      IF "!RUN_CMAKE!" == "1" (
        IF NOT EXIST CMakeFiles (
          MD CMakeFiles
        )
        ECHO %%i > CMakeFiles\cpp_hash.txt
      )
    )
    SET /A C=C+1
  )
  DEL cpp_hash.txt
)
IF "!RUN_CMAKE!" == "1" (
  cmake -S !DIRECTORY! -DD=!DEPENDENCIES!
)
ENDLOCAL

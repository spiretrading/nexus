@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET ROOT=%cd%
IF NOT EXIST build.bat (
  >build.bat ECHO @ECHO OFF
  >>build.bat ECHO CALL "%~dp0build.bat" %%*
)
IF NOT EXIST configure.bat (
  >configure.bat ECHO @ECHO OFF
  >>configure.bat ECHO CALL "%~dp0configure.bat" %%*
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
IF ERRORLEVEL 1 (
  ECHO Error: setup.bat failed.
  POPD
  EXIT /B 1
)
POPD
IF NOT "!DEPENDENCIES!" == "!ROOT!\Dependencies" (
  IF EXIST Dependencies (
    RD /S /Q Dependencies
  )
  mklink /j Dependencies "!DEPENDENCIES!" >NUL
)
SET RUN_CMAKE=
IF NOT EXIST CMakeFiles (
  MD CMakeFiles
  SET RUN_CMAKE=1
)
SET CMAKE_FILES=!ROOT!\cmake_files.txt
TYPE "!DIRECTORY!CMakeLists.txt" > "!CMAKE_FILES!"
FOR %%F IN (!DIRECTORY!Config\*.cmake) DO TYPE "%%F" >> "!CMAKE_FILES!"
PUSHD "!DIRECTORY!Config"
FOR /R %%F IN (*) DO (
  IF "%%~nxF"=="CMakeLists.txt" TYPE "%%F" >> "!CMAKE_FILES!"
)
POPD
FOR /F "skip=1" %%H IN ('certutil -hashfile "!CMAKE_FILES!" SHA256') DO (
  IF NOT DEFINED CMAKE_HASH SET CMAKE_HASH=%%H
)
DEL "!CMAKE_FILES!"
SET CMAKE_FILES=
IF EXIST CMakeFiles\cmake_hash.txt (
  SET /P CACHED_CMAKE_HASH=<CMakeFiles\cmake_hash.txt
  IF NOT "!CMAKE_HASH!"=="!CACHED_CMAKE_HASH!" SET RUN_CMAKE=1
) ELSE (
  SET RUN_CMAKE=1
)
IF "!RUN_CMAKE!" == "1" (
  >CMakeFiles\cmake_hash.txt ECHO !CMAKE_HASH!
)
SET CMAKE_HASH=
SET CACHED_CMAKE_HASH=
IF EXIST "!DIRECTORY!Include" (
  DIR /a-d /b /s "!DIRECTORY!Include\*" > hpp_hash.txt
  FOR /F "skip=1" %%H IN ('certutil -hashfile hpp_hash.txt SHA256') DO (
    IF NOT DEFINED HPP_HASH SET HPP_HASH=%%H
  )
  DEL hpp_hash.txt
  IF EXIST CMakeFiles\hpp_hash.txt (
    SET /P CACHED_HPP_HASH=<CMakeFiles\hpp_hash.txt
    IF NOT "!HPP_HASH!"=="!CACHED_HPP_HASH!" SET RUN_CMAKE=1
  ) ELSE (
    SET RUN_CMAKE=1
  )
  IF "!RUN_CMAKE!" == "1" (
    >CMakeFiles\hpp_hash.txt ECHO !HPP_HASH!
  )
  SET HPP_HASH=
  SET CACHED_HPP_HASH=
)
IF EXIST "!DIRECTORY!Source" (
  DIR /a-d /b /s "!DIRECTORY!Source\*" > cpp_hash.txt
  FOR /F "skip=1" %%H IN ('certutil -hashfile cpp_hash.txt SHA256') DO (
    IF NOT DEFINED CPP_HASH SET CPP_HASH=%%H
  )
  DEL cpp_hash.txt
  IF EXIST CMakeFiles\cpp_hash.txt (
    SET /P CACHED_CPP_HASH=<CMakeFiles\cpp_hash.txt
    IF NOT "!CPP_HASH!"=="!CACHED_CPP_HASH!" SET RUN_CMAKE=1
  ) ELSE (
    SET RUN_CMAKE=1
  )
  IF "!RUN_CMAKE!" == "1" (
    >CMakeFiles\cpp_hash.txt ECHO !CPP_HASH!
  )
  SET CPP_HASH=
  SET CACHED_CPP_HASH=
)
IF "!RUN_CMAKE!" == "1" (
  cmake -S !DIRECTORY! -DD=!DEPENDENCIES!
  IF ERRORLEVEL 1 (
    ECHO Error: CMake configuration failed.
    EXIT /B 1
  )
)
ENDLOCAL

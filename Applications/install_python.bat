@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET "DIRECTORY=%cd%\..\Nexus\Libraries"
SET "CONFIG=%~1"
IF "!CONFIG!"=="" SET "CONFIG=Release"
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
FOR %%F IN (aspen.pyd beam.pyd nexus.pyd) DO (
  IF NOT EXIST "!DIRECTORY!\!CONFIG!\%%F" (
    ECHO Error: Source file "!DIRECTORY!\!CONFIG!\%%F" not found.
    EXIT /B 1
  )
)
SET "PYTHON_PATH="
FOR /F "delims=" %%P IN ('python -m site --user-site 2^>NUL') DO (
  SET "PYTHON_PATH=%%P"
)
IF "!PYTHON_PATH!"=="" (
  ECHO Error: Unable to retrieve Python user-site path.
  EXIT /B 1
)
IF NOT EXIST "!PYTHON_PATH!" (
  MKDIR "!PYTHON_PATH!"
  IF ERRORLEVEL 1 (
    ECHO Error: Unable to create directory "!PYTHON_PATH!".
    EXIT /B 1
  )
)
FOR %%F IN (aspen.pyd beam.pyd nexus.pyd) DO (
  COPY /Y "!DIRECTORY!\!CONFIG!\%%F" "!PYTHON_PATH!\" >NUL
  IF ERRORLEVEL 1 (
    ECHO Error: Failed to copy "%%F" to "!PYTHON_PATH!".
    EXIT /B 1
  )
)
EXIT /B 0

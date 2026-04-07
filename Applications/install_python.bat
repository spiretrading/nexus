@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET CONFIG=%~1
IF "!CONFIG!" == "" SET CONFIG=Release
IF /I "!CONFIG!"=="release" SET CONFIG=Release
IF /I "!CONFIG!"=="debug" SET CONFIG=Debug
IF /I "!CONFIG!"=="relwithdebinfo" SET CONFIG=RelWithDebInfo
IF /I "!CONFIG!"=="minsizerel" SET CONFIG=MinSizeRel
SET PYTHON_PATH=
FOR /F "delims=" %%i IN ('python -m site --user-site 2^>NUL') DO SET PYTHON_PATH=%%i
IF "!PYTHON_PATH!" == "" (
  ECHO Error: Unable to retrieve Python user-site path.
  EXIT /B 1
)
PUSHD ..\Nexus\Dependencies\Beam\Applications
CALL install_python.bat %*
POPD
IF NOT EXIST "!PYTHON_PATH!" (
  MKDIR "!PYTHON_PATH!"
  IF ERRORLEVEL 1 (
    ECHO Error: Unable to create directory "!PYTHON_PATH!".
    EXIT /B 1
  )
)
IF NOT EXIST "..\Nexus\Libraries\!CONFIG!\nexus.pyd" (
  ECHO Error: Source file "..\Nexus\Libraries\!CONFIG!\nexus.pyd" not found.
  EXIT /B 1
)
COPY "..\Nexus\Libraries\!CONFIG!\nexus.pyd" "!PYTHON_PATH!" >NUL
IF ERRORLEVEL 1 (
  ECHO Error: Failed to copy "nexus.pyd" to "!PYTHON_PATH!".
  EXIT /B 1
)
EXIT /B 0

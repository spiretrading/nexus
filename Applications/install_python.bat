@ECHO OFF
IF "%~1" == "" (
  SET CONFIG=Release
) ELSE (
  SET CONFIG=%~1
)
SET PYTHON_PATH=
FOR /f "delims=" %%i IN ('python -m site --user-site 2^>nul') DO SET PYTHON_PATH=%%i
IF "%PYTHON_PATH%" == "" (
  ECHO Error: Unable to retrieve Python user-site path.
  EXIT /B 1
)
PUSHD ..\Nexus\Dependencies\Beam\Applications
CALL install_python.bat %*
POPD
IF NOT EXIST "%PYTHON_PATH%" (
  MKDIR "%PYTHON_PATH%"
  IF %ERRORLEVEL% GEQ 1 (
    ECHO Error: Unable to create directory "%PYTHON_PATH%".
    EXIT /B 1
  )
)
IF NOT EXIST "..\Nexus\Libraries\%CONFIG%\nexus.pyd" (
  ECHO Error: Source file "..\Nexus\Libraries\%CONFIG%\nexus.pyd" not found.
  EXIT /B 1
)
COPY "..\Nexus\Libraries\%CONFIG%\nexus.pyd" "%PYTHON_PATH%" >nul
IF %ERRORLEVEL% GEQ 1 (
  ECHO Error: Failed to copy "nexus.pyd" to "%PYTHON_PATH%".
  EXIT /B 1
)
EXIT /B 0

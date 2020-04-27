IF "%1" == "" (
  SET CONFIG=Release
) ELSE (
  SET CONFIG=%1
)
FOR /f "delims=" %%i IN ('python -m site --user-site') DO SET PYTHON_PATH="%%i"
CALL ..\Nexus\Dependencies\Beam\Applications\install_python.bat %*
MD %PYTHON_PATH%\nexus
COPY Python\__init__.py %PYTHON_PATH%\nexus
COPY ..\Nexus\Libraries\%CONFIG%\_nexus.pyd %PYTHON_PATH%\nexus

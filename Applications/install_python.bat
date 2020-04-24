IF "%1" == "" (
  SET CONFIG=Release
) ELSE (
  SET CONFIG=%1
)
FOR /f "delims=" %%i IN ('python -m site --user-site') DO SET PYTHON_PATH="%%i"
COPY ..\Nexus\Dependencies\aspen\Libraries\%CONFIG%\aspen.pyd %PYTHON_PATH%
MD %PYTHON_PATH%\beam
COPY ..\Nexus\Dependencies\Beam\Applications\Python\__init__.py ^
  %PYTHON_PATH%\beam
COPY ..\Nexus\Dependencies\Beam\Beam\Libraries\%CONFIG%\_beam.pyd ^
  %PYTHON_PATH%\beam
MD %PYTHON_PATH%\nexus
COPY Python\__init__.py %PYTHON_PATH%\nexus
COPY ..\Nexus\Libraries\%CONFIG%\_nexus.pyd %PYTHON_PATH%\nexus

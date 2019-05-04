@ECHO OFF
SETLOCAL
SET ROOT=%cd%
CALL "%~dp0..\setup.bat"
IF NOT EXIST library (
  MD library
  PUSHD library
  CALL "%~dp0..\library\configure.bat" -DD="%ROOT%"
  POPD
)
ENDLOCAL

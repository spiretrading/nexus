SETLOCAL
IF NOT %1 == "Debug" (
  SET PROD_ENV='1'
)
CALL %~dp0../../source/build/windows/build.bat %*
CALL %~dp0../../tests/build/windows/build.bat %*
CALL %~dp0../../application/build/windows/build.bat %*
ENDLOCAL

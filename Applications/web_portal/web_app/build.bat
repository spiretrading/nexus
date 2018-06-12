SETLOCAL
CALL %~dp0/tests/build/windows/build.bat %*
CALL %~dp0/application/build/windows/build.bat %*
ENDLOCAL

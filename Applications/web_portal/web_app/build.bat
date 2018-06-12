ECHO OFF
SETLOCAL
CALL %~dp0/library/build.bat %*
CALL %~dp0/tests/build.bat %*
CALL %~dp0/application/build.bat %*
ENDLOCAL

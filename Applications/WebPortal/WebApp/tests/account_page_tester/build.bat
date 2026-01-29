@ECHO OFF
CALL "%~dp0..\..\application\build.bat" -D="%~dp0" %*
EXIT /B %ERRORLEVEL%

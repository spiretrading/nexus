@ECHO OFF
CALL "%~dp0..\..\application\configure.bat" -D="%~dp0" %*
EXIT /B %ERRORLEVEL%

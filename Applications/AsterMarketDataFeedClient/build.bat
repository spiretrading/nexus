@ECHO OFF
CALL "%~dp0..\..\Nexus\build.bat" -D "%~dp0" %*
EXIT /B %ERRORLEVEL%

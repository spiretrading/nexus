@ECHO OFF
CALL "%~dp0..\..\Nexus\configure.bat" -D "%~dp0" %*
EXIT /B %ERRORLEVEL%

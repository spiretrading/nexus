@ECHO OFF
CALL "%~dp0..\..\application\configure.bat" %*
EXIT /B %ERRORLEVEL%

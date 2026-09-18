@ECHO OFF
node "%~dp0build.js" configure %*
EXIT /B %ERRORLEVEL%

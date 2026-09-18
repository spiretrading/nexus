@ECHO OFF
node "%~dp0build.js" build %*
EXIT /B %ERRORLEVEL%

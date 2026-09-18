@ECHO OFF
node "%~dp0build.js" setup %*
EXIT /B %ERRORLEVEL%

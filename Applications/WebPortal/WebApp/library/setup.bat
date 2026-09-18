@ECHO OFF
SETLOCAL DisableDelayedExpansion
SET ARGS=%*
node "%~dp0..\build.js" library setup %ARGS:\=/%
EXIT /B %ERRORLEVEL%

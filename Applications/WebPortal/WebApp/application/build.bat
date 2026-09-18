@ECHO OFF
SETLOCAL DisableDelayedExpansion
SET ARGS=%*
node "%~dp0..\build.js" application build %ARGS:\=/%
EXIT /B %ERRORLEVEL%

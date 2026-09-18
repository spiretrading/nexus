@ECHO OFF
SETLOCAL DisableDelayedExpansion
SET ARGS=%*
node "%~dp0..\build.js" application setup %ARGS:\=/%
EXIT /B %ERRORLEVEL%

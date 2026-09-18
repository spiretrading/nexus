@ECHO OFF
SETLOCAL DisableDelayedExpansion
SET ARGS=%*
node "%~dp0..\build.js" application configure %ARGS:\=/%
EXIT /B %ERRORLEVEL%

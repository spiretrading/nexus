@ECHO OFF
SETLOCAL DisableDelayedExpansion
SET ARGS=%*
node "%~dp0..\build.js" tests build %ARGS:\=/%
EXIT /B %ERRORLEVEL%

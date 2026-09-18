@ECHO OFF
SETLOCAL DisableDelayedExpansion
SET ARGS=%*
node "%~dp0..\build.js" library build %ARGS:\=/%
EXIT /B %ERRORLEVEL%

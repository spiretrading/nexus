@ECHO OFF
SETLOCAL DisableDelayedExpansion
SET ARGS=%*
node "%~dp0..\build.js" tests setup %ARGS:\=/%
EXIT /B %ERRORLEVEL%

@ECHO OFF
SETLOCAL DisableDelayedExpansion
SET ARGS=%*
node "%~dp0..\build.js" tests configure %ARGS:\=/%
EXIT /B %ERRORLEVEL%

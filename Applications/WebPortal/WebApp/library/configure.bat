@ECHO OFF
SETLOCAL DisableDelayedExpansion
SET ARGS=%*
node "%~dp0..\build.js" library configure %ARGS:\=/%
EXIT /B %ERRORLEVEL%

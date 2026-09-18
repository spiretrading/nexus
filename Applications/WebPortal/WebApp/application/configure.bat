@ECHO OFF
SETLOCAL DisableDelayedExpansion
SET ARGS=%*
IF DEFINED ARGS SET ARGS=%ARGS:\=/%
node "%~dp0..\build.js" application configure %ARGS%
EXIT /B %ERRORLEVEL%

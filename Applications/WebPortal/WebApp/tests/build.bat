@ECHO OFF
SETLOCAL DisableDelayedExpansion
SET ARGS=%*
IF DEFINED ARGS SET ARGS=%ARGS:\=/%
node "%~dp0..\build.js" tests build %ARGS%
EXIT /B %ERRORLEVEL%

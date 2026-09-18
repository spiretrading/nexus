@ECHO OFF
SETLOCAL DisableDelayedExpansion
SET ARGS=%*
IF DEFINED ARGS SET ARGS=%ARGS:\=/%
node "%~dp0..\build.js" library build %ARGS%
EXIT /B %ERRORLEVEL%

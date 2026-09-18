@ECHO OFF
SETLOCAL DisableDelayedExpansion
SET ARGS=%*
IF DEFINED ARGS SET ARGS=%ARGS:\=/%
node "%~dp0build.js" portal build %ARGS%
EXIT /B %ERRORLEVEL%

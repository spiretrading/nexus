@ECHO OFF
SETLOCAL DisableDelayedExpansion
SET ARGS=%*
node "%~dp0build.js" portal build %ARGS:\=/%
EXIT /B %ERRORLEVEL%

@ECHO OFF
SETLOCAL DisableDelayedExpansion
SET ARGS=%*
node "%~dp0build.js" portal configure %ARGS:\=/%
EXIT /B %ERRORLEVEL%

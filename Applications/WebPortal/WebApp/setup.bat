@ECHO OFF
SETLOCAL DisableDelayedExpansion
SET ARGS=%*
node "%~dp0build.js" portal setup %ARGS:\=/%
EXIT /B %ERRORLEVEL%

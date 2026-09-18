@ECHO OFF
SETLOCAL DisableDelayedExpansion
SET ARGS=%*
node "%~dp0..\..\build.js" application configure -D="%~dp0." %ARGS:\=/%
EXIT /B %ERRORLEVEL%

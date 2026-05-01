@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET "ARG=%~1"
IF /I "!ARG!"=="clean" (
  RD /S /Q "Dependencies\library" 2>NUL
  RD /S /Q "Dependencies\WebApi" 2>NUL
  DEL "Dependencies\cache_files\nexus_webapi.txt" 2>NUL
  DEL "Dependencies\cache_files\nexus_webapp.txt" 2>NUL
)
CALL "%~dp0..\..\Nexus\build.bat" -D "%~dp0" %*
EXIT /B %ERRORLEVEL%

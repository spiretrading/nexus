@ECHO OFF
SETLOCAL
IF /I "%~1"=="help" GOTO Help
IF /I "%~1"=="-h" GOTO Help
IF "%~1"=="" (
  ECHO Error: Missing version parameter.
  EXIT /B 1
)
SET "COMPILER=%ProgramFiles(x86)%\NSIS\makensis.exe"
IF NOT EXIST "%COMPILER%" (
  ECHO Error: NSIS is required to build the installer.
  EXIT /B 1
)
"%COMPILER%" /NOCD "/DBUILD=%~1" "%~dp0InstallerScript.nsi"
EXIT /B %ERRORLEVEL%

:Help
ECHO Usage: build [version]
ECHO   version - The current version number.
EXIT /B 0

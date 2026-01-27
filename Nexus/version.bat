@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET "APP_NAME=%~1"
IF "!APP_NAME!"=="" (
  ECHO Error: Application name required.
  EXIT /B 1
)
IF NOT EXIST Version.hpp (
  COPY NUL Version.hpp >NUL
)
FOR /F "usebackq tokens=*" %%a IN (
    `git --git-dir=%~dp0../.git rev-list --count --first-parent HEAD`) DO (
  SET "VERSION=%%a"
)
FINDSTR "!VERSION!" Version.hpp >NUL
IF ERRORLEVEL 1 (
  >Version.hpp ECHO #define !APP_NAME!_VERSION "!VERSION!"
)
EXIT /B 0

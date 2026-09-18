@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET "APP_NAME=%~1"
IF "!APP_NAME!"=="" (
  ECHO Error: Application name required.
  EXIT /B 1
)
SET "VERSION="
SET "MASTER_REF="
SET "BASE_REF="
SET "BASE="
SET "EXTRA="
FOR %%r IN (master origin/master) DO (
  IF NOT DEFINED MASTER_REF (
    git -C "%~dp0.." rev-parse --verify --quiet %%r >NUL 2>&1
    IF NOT ERRORLEVEL 1 SET "MASTER_REF=%%r"
  )
)
IF DEFINED MASTER_REF (
  FOR /F "usebackq tokens=*" %%m IN (
      `git -C "%~dp0.." merge-base HEAD !MASTER_REF!`
  ) DO (
    SET "BASE_REF=%%m"
  )
  FOR /F "usebackq tokens=*" %%a IN (
      `git -C "%~dp0.." rev-list --count --first-parent !BASE_REF!`
  ) DO (
    SET "BASE=%%a"
  )
  FOR /F "usebackq tokens=*" %%b IN (
      `git -C "%~dp0.." rev-list --count --no-merges !BASE_REF!..HEAD`
  ) DO (
    SET "EXTRA=%%b"
  )
  IF NOT DEFINED BASE_REF EXIT /B 1
  IF NOT DEFINED BASE EXIT /B 1
  IF NOT DEFINED EXTRA EXIT /B 1
  SET /A VERSION=BASE+EXTRA
) ELSE (
  FOR /F "usebackq tokens=*" %%a IN (
      `git -C "%~dp0.." rev-list --count --first-parent HEAD`) DO (
    SET "VERSION=%%a"
  )
)
IF NOT DEFINED VERSION EXIT /B 1
FINDSTR /L /X /C:"#define !APP_NAME!_VERSION \"!VERSION!\"" ^
  Version.hpp >NUL 2>&1
IF ERRORLEVEL 1 (
  (ECHO #define !APP_NAME!_VERSION "!VERSION!") >Version.hpp || EXIT /B 1
)
EXIT /B 0

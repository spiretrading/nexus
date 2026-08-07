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
SET "GIT_DIR=%~dp0../.git"
SET "MASTER_REF="
FOR %%r IN (master origin/master) DO IF NOT DEFINED MASTER_REF (
  git --git-dir=!GIT_DIR! rev-parse --verify --quiet %%r >NUL 2>&1
  IF NOT ERRORLEVEL 1 SET "MASTER_REF=%%r"
)
IF DEFINED MASTER_REF (
  FOR /F "usebackq tokens=*" %%m IN (
      `git --git-dir=!GIT_DIR! merge-base HEAD !MASTER_REF!`
  ) DO (
    SET "BASE_REF=%%m"
  )
  FOR /F "usebackq tokens=*" %%a IN (
      `git --git-dir=!GIT_DIR! rev-list --count --first-parent !BASE_REF!`
  ) DO (
    SET "BASE=%%a"
  )
  FOR /F "usebackq tokens=*" %%b IN (
      `git --git-dir=!GIT_DIR! rev-list --count --no-merges !BASE_REF!..HEAD`
  ) DO (
    SET "EXTRA=%%b"
  )
  SET /A VERSION=BASE+EXTRA
) ELSE (
  FOR /F "usebackq tokens=*" %%a IN (
      `git --git-dir=!GIT_DIR! rev-list --count --first-parent HEAD`) DO (
    SET "VERSION=%%a"
  )
)
FINDSTR "!VERSION!" Version.hpp >NUL
IF ERRORLEVEL 1 (
  >Version.hpp ECHO #define !APP_NAME!_VERSION "!VERSION!"
)
EXIT /B 0

@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET "ROOT=%cd%"
CALL :CheckCache "nexus_webapi"
IF ERRORLEVEL 1 EXIT /B 0
CALL :AddRepo "Beam" ^
  "https://www.github.com/spiretrading/beam" ^
  "96faba0c3b7dffe659399e959a599ffae765d1a1"
CALL :InstallRepos || EXIT /B 1
CALL :Commit
EXIT /B !ERRORLEVEL!
ENDLOCAL

:CheckCache
SET "CACHE_NAME=%~1"
SET "SETUP_HASH="
FOR /F "skip=1" %%H IN ('certutil -hashfile "%~dp0setup.bat" SHA256') DO (
  IF NOT DEFINED SETUP_HASH SET "SETUP_HASH=%%H"
)
IF EXIST "cache_files\!CACHE_NAME!.txt" (
  SET /P CACHED_HASH=<"cache_files\!CACHE_NAME!.txt"
  IF "!SETUP_HASH!"=="!CACHED_HASH!" EXIT /B 1
)
EXIT /B 0

:Commit
IF NOT EXIST cache_files (
  MD cache_files || EXIT /B 1
)
>"cache_files\!CACHE_NAME!.txt" ECHO !SETUP_HASH!
EXIT /B 0

:AddRepo
IF NOT DEFINED NEXT_REPO_INDEX SET "NEXT_REPO_INDEX=0"
SET "REPOS[%NEXT_REPO_INDEX%].NAME=%~1"
SET "REPOS[%NEXT_REPO_INDEX%].URL=%~2"
SET "REPOS[%NEXT_REPO_INDEX%].COMMIT=%~3"
SET "REPOS[%NEXT_REPO_INDEX%].BUILD=%~4"
SET /A NEXT_REPO_INDEX+=1
EXIT /B 0

:InstallRepos
SET "I=0"
:InstallReposLoop
IF NOT DEFINED REPOS[%I%].NAME EXIT /B 0
CALL :CloneOrUpdateRepo "!REPOS[%I%].NAME!" "!REPOS[%I%].URL!" ^
  "!REPOS[%I%].COMMIT!" "!REPOS[%I%].BUILD!" || EXIT /B 1
SET /A I+=1
GOTO InstallReposLoop

:CloneOrUpdateRepo
SET "REPO_NAME=%~1"
SET "REPO_URL=%~2"
SET "REPO_COMMIT=%~3"
SET "BUILD_LABEL=%~4"
SET "NEEDS_BUILD=0"
IF NOT EXIST "!REPO_NAME!" (
  git clone "!REPO_URL!" "!REPO_NAME!"
  IF ERRORLEVEL 1 (
    RD /S /Q "!REPO_NAME!" >NUL 2>NUL
    EXIT /B 1
  )
  PUSHD "!REPO_NAME!"
  git checkout "!REPO_COMMIT!"
  POPD
  SET "NEEDS_BUILD=1"
) ELSE (
  PUSHD "!REPO_NAME!"
  git merge-base --is-ancestor "!REPO_COMMIT!" HEAD
  IF ERRORLEVEL 1 (
    git checkout master
    git pull
    git checkout "!REPO_COMMIT!"
    SET "NEEDS_BUILD=1"
  )
  POPD
)
IF "!NEEDS_BUILD!"=="1" (
  IF DEFINED BUILD_LABEL (
    PUSHD "!REPO_NAME!"
    CALL !BUILD_LABEL!
    SET "BUILD_RESULT=!ERRORLEVEL!"
    POPD
    IF NOT "!BUILD_RESULT!"=="0" EXIT /B !BUILD_RESULT!
  )
)
EXIT /B 0

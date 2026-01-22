@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET ROOT=%cd%
SET DIRECTORY=%~dp0
SET EXIT_STATUS=0
IF NOT EXIST configure.bat (
  >configure.bat ECHO @ECHO OFF
  >>configure.bat ECHO CALL "%~dp0configure.bat" %%*
)
IF NOT EXIST build.bat (
  >build.bat ECHO @ECHO OFF
  >>build.bat ECHO CALL "%~dp0build.bat" %%*
)
SET ARGS=%*
SET BUILD_TEMP=!ROOT!\_build_tmp
IF EXIST "!BUILD_TEMP!" RD /S /Q "!BUILD_TEMP!"
MD "!BUILD_TEMP!"
CALL:build_parallel account_directory_page_tester
CALL:build_parallel account_page_tester
CALL:build_parallel compliance_page_tester
CALL:build_parallel create_account_page_tester
CALL:build_parallel dashboard_page_tester
CALL:build_parallel entitlements_page_tester
CALL:build_parallel group_info_page_tester
CALL:build_parallel group_page_tester
CALL:build_parallel loading_page_tester
CALL:build_parallel login_page_tester
CALL:build_parallel mock
CALL:build_parallel page_not_found_page_tester
CALL:build_parallel profile_page_tester
CALL:build_parallel risk_page_tester
CALL:build_parallel scratch
:wait_loop
SET RUNNING=0
FOR %%F IN ("!BUILD_TEMP!\*.running") DO SET RUNNING=1
IF !RUNNING! EQU 1 (
  timeout /t 1 /nobreak >NUL
  GOTO wait_loop
)
FOR %%F IN ("!BUILD_TEMP!\*.log") DO (
  ECHO.
  ECHO ============================================================
  ECHO %%~nF
  ECHO ============================================================
  TYPE "%%F"
)
FOR %%F IN ("!BUILD_TEMP!\*.failed") DO (
  SET EXIT_STATUS=1
)
RD /S /Q "!BUILD_TEMP!"
EXIT /B !EXIT_STATUS!
ENDLOCAL

:build_parallel
SET PROJECT=%~1
IF NOT EXIST "!PROJECT!" (
  MD "!PROJECT!"
)
>"!BUILD_TEMP!\!PROJECT!.running" ECHO !PROJECT!
START /B cmd /c "PUSHD "!ROOT!\!PROJECT!" && CALL "!DIRECTORY!!PROJECT!\build.bat" -DD="!ROOT!\..\library\Dependencies" !ARGS! && DEL "!BUILD_TEMP!\!PROJECT!.running" || (DEL "!BUILD_TEMP!\!PROJECT!.running" & ECHO failed > "!BUILD_TEMP!\!PROJECT!.failed")" >"!BUILD_TEMP!\!PROJECT!.log" 2>&1
EXIT /B 0

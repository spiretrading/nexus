@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET ROOT=%cd%
SET EXIT_STATUS=0
IF NOT EXIST configure.bat (
  >configure.bat ECHO @ECHO OFF
  >>configure.bat ECHO CALL "%~dp0configure.bat" %%*
)
IF NOT EXIST build.bat (
  >build.bat ECHO @ECHO OFF
  >>build.bat ECHO CALL "%~dp0build.bat" %%*
)
CALL:configure account_directory_page_tester %*
CALL:configure account_page_tester %*
CALL:configure compliance_page_tester %*
CALL:configure create_account_page_tester %*
CALL:configure dashboard_page_tester %*
CALL:configure entitlements_page_tester %*
CALL:configure group_info_page_tester %*
CALL:configure group_page_tester %*
CALL:configure loading_page_tester %*
CALL:configure login_page_tester %*
CALL:configure mock %*
CALL:configure page_not_found_page_tester %*
CALL:configure profile_page_tester %*
CALL:configure risk_page_tester %*
CALL:configure scratch %*
EXIT /B !EXIT_STATUS!
ENDLOCAL

:configure
IF NOT EXIST "%~1" (
  MD "%~1"
)
PUSHD "%~1"
CALL "%~dp0%~1\configure.bat" -DD="!ROOT!\..\library\Dependencies" %~2 %~3 %~4 %~5 %~6 %~7
IF ERRORLEVEL 1 SET EXIT_STATUS=1
POPD
EXIT /B 0

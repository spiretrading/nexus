@ECHO OFF
SETLOCAL
SET ROOT=%cd%
IF NOT EXIST configure.bat (
  ECHO @ECHO OFF > configure.bat
  ECHO CALL "%~dp0configure.bat" %%* >> configure.bat
)
IF NOT EXIST build.bat (
  ECHO @ECHO OFF > build.bat
  ECHO CALL "%~dp0build.bat" %%* >> build.bat
)
CALL:build account_directory_page_tester %*
CALL:build account_page_tester %*
CALL:build compliance_page_tester %*
CALL:build create_account_page_tester %*
CALL:build dashboard_page_tester %*
CALL:build entitlements_page_tester %*
CALL:build group_info_page_tester %*
CALL:build group_page_tester %*
CALL:build loading_page_tester %*
CALL:build login_page_tester %*
CALL:build mock %*
CALL:build page_not_found_page_tester %*
CALL:build profile_page_tester %*
CALL:build risk_page_tester %*
CALL:build scratch %*
ENDLOCAL
EXIT /B %ERRORLEVEL%

:build
IF NOT EXIST "%~1" (
  MD "%~1"
)
PUSHD "%~1"
CALL "%~dp0%~1\build.bat" -DD="%ROOT%\..\library\Dependencies" %~2 %~3 %~4 %~5 %~6 %~7
POPD
EXIT /B 0

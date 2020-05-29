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
CALL:configure account_directory_page_tester %*
CALL:configure account_page_tester %*
CALL:configure compliance_page_tester %*
CALL:configure create_account_page_tester %*
CALL:configure dashboard_page_tester %*
CALL:configure entitlements_page_tester %*
CALL:configure group_info_page_tester %*
CALL:configure loading_page_tester %*
CALL:configure login_page_tester %*
CALL:configure profile_page_tester %*
CALL:configure risk_page_tester %*
CALL:configure scratch %*
ENDLOCAL
EXIT /B %ERRORLEVEL%

:configure
IF NOT EXIST "%~1" (
  MD "%~1"
)
PUSHD "%~1"
CALL "%~dp0%~1\configure.bat" -DD="%ROOT%\Dependencies" %~2 %~3 %~4 %~5 %~6 %~7
POPD
EXIT /B 0

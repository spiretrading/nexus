@ECHO OFF
SETLOCAL
SET ROOT=%cd%
CALL:build account_directory_page_tester %*
REM CALL:build account_page_tester %*
REM CALL:build create_account_page_tester %*
REM CALL:build dashboard_page_tester %*
REM CALL:build entitlements_page_tester %*
REM CALL:build loading_page_tester %*
REM CALL:build login_page_tester %*
REM CALL:build profile_page_tester %*
REM CALL:build risk_page_tester %*
REM CALL:build scratch %*
ENDLOCAL
EXIT /B %ERRORLEVEL%

:build
IF NOT EXIST "%~1" (
  MD "%~1"
)
PUSHD "%~1"
CALL "%~dp0%~1\build.bat" %~2 %~3 %~4 %~5 %~6 %~7
POPD
EXIT /B 0

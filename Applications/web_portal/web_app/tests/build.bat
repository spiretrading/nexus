ECHO OFF
SETLOCAL
CALL %~dp0login_page_tester/build.bat %*
CALL %~dp0dashboard_page_tester/build.bat %*
ENDLOCAL

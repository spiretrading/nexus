ECHO OFF
SETLOCAL
CALL %~dp0account_page_tester/build.bat %*
CALL %~dp0login_page_tester/build.bat %*
CALL %~dp0dashboard_page_tester/build.bat %*
CALL %~dp0risk_page_tester/build.bat %*
CALL %~dp0scratch/build.bat %*
ENDLOCAL

@ECHO OFF
CALL "%~dp0..\..\Nexus\version.bat" RISK_SERVER
EXIT /B %ERRORLEVEL%

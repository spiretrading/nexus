@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET "ROOT=%cd%"
SET "EXIT_STATUS=0"
IF NOT EXIST configure.bat (
  >configure.bat ECHO @ECHO OFF
  >>configure.bat ECHO CALL "%~dp0configure.bat" %%*
)
IF NOT EXIST build.bat (
  >build.bat ECHO @ECHO OFF
  >>build.bat ECHO CALL "%~dp0build.bat" %%*
)
CALL :Configure Nexus %*
CALL :Configure WebApi %*
CALL :Configure Applications\AdministrationServer %*
CALL :Configure Applications\ChartingServer %*
CALL :Configure Applications\ComplianceServer %*
CALL :Configure Applications\DefinitionsServer %*
CALL :Configure Applications\Lollipop %*
CALL :Configure Applications\MarketDataRelayServer %*
CALL :Configure Applications\MarketDataServer %*
CALL :Configure Applications\ReplayMarketDataFeedClient %*
CALL :Configure Applications\RiskServer %*
CALL :Configure Applications\Scratch %*
CALL :Configure Applications\SimulationMarketDataFeedClient %*
CALL :Configure Applications\SimulationOrderExecutionServer %*
CALL :Configure Applications\Spire %*
CALL :Configure Applications\WebPortal\WebApp %*
CALL :Configure Applications\WebPortal %*
EXIT /B !EXIT_STATUS!
ENDLOCAL

:Configure
IF NOT EXIST "%~1" (
  MD "%~1"
)
PUSHD "%~1"
CALL "%~dp0%~1\configure.bat" -DD="!ROOT!\Nexus\Dependencies" %~2 %~3 %~4 %~5 %~6 %~7
IF ERRORLEVEL 1 SET "EXIT_STATUS=1"
POPD
EXIT /B 0

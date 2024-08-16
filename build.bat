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
CALL:build Nexus %*
CALL:build WebApi %*
CALL:build Applications\AdministrationServer %*
CALL:build Applications\ChartingServer %*
CALL:build Applications\ComplianceServer %*
CALL:build Applications\DefinitionsServer %*
CALL:build Applications\Lollipop %*
CALL:build Applications\MarketDataRelayServer %*
CALL:build Applications\MarketDataServer %*
CALL:build Applications\ReplayMarketDataFeedClient %*
CALL:build Applications\RiskServer %*
CALL:build Applications\Scratch %*
CALL:build Applications\SimulationMarketDataFeedClient %*
CALL:build Applications\SimulationOrderExecutionServer %*
CALL:build Applications\Spire %*
CALL:build Applications\TelemetryServer %*
CALL:build Applications\WebPortal\WebApp %*
CALL:build Applications\WebPortal %*
ENDLOCAL
EXIT /B %ERRORLEVEL%

:build
IF NOT EXIST "%~1" (
  MD "%~1"
)
PUSHD "%~1"
CALL "%~dp0%~1\build.bat" -DD="%ROOT%\Nexus\Dependencies" %~2 %~3 %~4 %~5 %~6 %~7
POPD
EXIT /B 0

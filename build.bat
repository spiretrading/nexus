@ECHO OFF
SETLOCAL
SET ROOT=%cd%
CALL:build Nexus %*
CALL:build WebApi %*
CALL:build Applications\AdministrationServer %*
CALL:build Applications\AsxItchMarketDataFeedClient %*
CALL:build Applications\ChartingServer %*
CALL:build Applications\ChiaMarketDataFeedClient %*
CALL:build Applications\ComplianceServer %*
CALL:build Applications\CseMarketDataFeedClient %*
CALL:build Applications\CtaMarketDataFeedClient %*
CALL:build Applications\DefinitionsServer %*
CALL:build Applications\MarketDataRelayServer %*
CALL:build Applications\MarketDataServer %*
CALL:build Applications\ReplayMarketDataFeedClient %*
CALL:build Applications\RiskServer %*
CALL:build Applications\SimulationMarketDataFeedClient %*
CALL:build Applications\SimulationOrderExecutionServer %*
CALL:build Applications\Spire %*
CALL:build Applications\TmxIpMarketDataFeedClient %*
CALL:build Applications\TmxTl1MarketDataFeedClient %*
CALL:build Applications\UtpMarketDataFeedClient %*
CALL:build Applications\WebPortal %*
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

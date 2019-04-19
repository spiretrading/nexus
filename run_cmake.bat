@ECHO OFF
SETLOCAL
SET ROOT=%cd%
IF NOT EXIST run_cmake.bat (
  ECHO CALL "%~dp0run_cmake.bat" %%* > run_cmake.bat
)
IF NOT EXIST build.bat (
  ECHO CALL "%~dp0build.bat" %%* > build.bat
)
CALL:run_cmake Nexus %*
CALL:run_cmake Applications\AdministrationServer %*
CALL:run_cmake Applications\AsxItchMarketDataFeedClient %*
CALL:run_cmake Applications\ChartingServer %*
CALL:run_cmake Applications\ChiaMarketDataFeedClient %*
CALL:run_cmake Applications\ComplianceServer %*
CALL:run_cmake Applications\CseMarketDataFeedClient %*
CALL:run_cmake Applications\CtaMarketDataFeedClient %*
CALL:run_cmake Applications\DefinitionsServer %*
CALL:run_cmake Applications\MarketDataRelayServer %*
CALL:run_cmake Applications\MarketDataServer %*
CALL:run_cmake Applications\ReplayMarketDataFeedClient %*
CALL:run_cmake Applications\RiskServer %*
CALL:run_cmake Applications\SimulationMarketDataFeedClient %*
CALL:run_cmake Applications\SimulationOrderExecutionServer %*
CALL:run_cmake Applications\Spire %*
CALL:run_cmake Applications\TmxIpMarketDataFeedClient %*
CALL:run_cmake Applications\TmxTl1MarketDataFeedClient %*
CALL:run_cmake Applications\UtpMarketDataFeedClient %*
CALL:run_cmake Applications\WebPortal %*
ENDLOCAL
EXIT /B %ERRORLEVEL%

:run_cmake
IF NOT EXIST "%~1" (
  MD "%~1"
)
PUSHD "%~1"
IF NOT EXIST run_cmake.bat (
  ECHO CALL "%~dp0%~1\run_cmake.bat" %%* > run_cmake.bat
)
CALL "%~dp0%~1\run_cmake.bat" -DD="%ROOT%\Dependencies" %~2 %~3 %~4 %~5 %~6 %~7
POPD
EXIT /B 0

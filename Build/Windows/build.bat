SETLOCAL
IF [%1] == [] (
  SET config=Release
) ELSE (
  SET config="%1"
)
CALL %~dp0../../Nexus/Build/Windows/build.bat %config%
CALL %~dp0../../Applications/AdministrationServer/Build/Windows/build.bat %config%
CALL %~dp0../../Applications/AsxItchMarketDataFeedClient/Build/Windows/build.bat %config%
CALL %~dp0../../Applications/ChartingServer/Build/Windows/build.bat %config%
CALL %~dp0../../Applications/ChiaMarketDataFeedClient/Build/Windows/build.bat %config%
CALL %~dp0../../Applications/ClientWebPortal/Build/Windows/build.bat %config%
CALL %~dp0../../Applications/ComplianceServer/Build/Windows/build.bat %config%
CALL %~dp0../../Applications/CseMarketDataFeedClient/Build/Windows/build.bat %config%
CALL %~dp0../../Applications/CtaMarketDataFeedClient/Build/Windows/build.bat %config%
CALL %~dp0../../Applications/DefinitionsServer/Build/Windows/build.bat %config%
CALL %~dp0../../Applications/MarketDataRelayServer/Build/Windows/build.bat %config%
CALL %~dp0../../Applications/MarketDataServer/Build/Windows/build.bat %config%
CALL %~dp0../../Applications/RiskServer/Build/Windows/build.bat %config%
CALL %~dp0../../Applications/SimulationMarketDataFeedClient/Build/Windows/build.bat %config%
CALL %~dp0../../Applications/SimulationOrderExecutionServer/Build/Windows/build.bat %config%
CALL %~dp0../../Applications/Spire/Build/Windows/build.bat %config%
CALL %~dp0../../Applications/TmxIpMarketDataFeedClient/Build/Windows/build.bat %config%
CALL %~dp0../../Applications/TmxTl1MarketDataFeedClient/Build/Windows/build.bat %config%
CALL %~dp0../../Applications/UtpMarketDataFeedClient/Build/Windows/build.bat %config%
ENDLOCAL

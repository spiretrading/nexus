@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET "ROOT=%cd%"
SET "EXIT_STATUS=0"
SET "NEXUS_SETUP_DIRECTORY="
CALL :ParseArgs %* || EXIT /B 1
IF NOT EXIST configure.bat (
  >configure.bat ECHO @ECHO OFF
  >>configure.bat ECHO CALL "%~dp0configure.bat" %%*
)
IF NOT EXIST build.bat (
  >build.bat ECHO @ECHO OFF
  >>build.bat ECHO CALL "%~dp0build.bat" %%*
)
CALL :Configure Nexus %*
IF !EXIT_STATUS! NEQ 0 EXIT /B !EXIT_STATUS!
SET "NEXUS_SETUP_DIRECTORY=!DEPENDENCIES!"
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

:ParseArgs
SET "DEPENDENCIES=!ROOT!\Nexus\Dependencies"
SET "ARGS="
SET "IS_DEPENDENCY="
:ParseArgsLoop
SET "ARG=%~1"
IF "!ARG!"=="" (
  IF "!IS_DEPENDENCY!"=="1" (
    ECHO Error: -DD requires a path argument.
    EXIT /B 1
  )
  GOTO ParseArgsDone
)
IF "!IS_DEPENDENCY!"=="1" (
  SET "DEPENDENCIES=!ARG!"
  SET "IS_DEPENDENCY="
) ELSE IF "!ARG!"=="-DD" (
  SET "IS_DEPENDENCY=1"
) ELSE IF "!ARG:~0,4!"=="-DD=" (
  SET "DEPENDENCIES=!ARG:~4!"
  IF "!DEPENDENCIES!"=="" (
    ECHO Error: -DD requires a path argument.
    EXIT /B 1
  )
) ELSE (
  SET ARGS=!ARGS! "%~1"
)
SHIFT
GOTO ParseArgsLoop
:ParseArgsDone
FOR %%D IN ("!DEPENDENCIES!") DO (
  SET "DEPENDENCIES=%%~fD"
)
EXIT /B 0

:Configure
IF NOT EXIST "%~1" (
  MD "%~1"
)
PUSHD "%~1"
CALL "%~dp0%~1\configure.bat" -DD="!DEPENDENCIES!" !ARGS!
IF ERRORLEVEL 1 SET "EXIT_STATUS=1"
POPD
EXIT /B 0

@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET "ROOT=%cd%"
SET "DIRECTORY=%~dp0"
SET "EXIT_STATUS=0"
IF NOT EXIST configure.bat (
  >configure.bat ECHO @ECHO OFF
  >>configure.bat ECHO CALL "%~dp0configure.bat" %%*
)
IF NOT EXIST build.bat (
  >build.bat ECHO @ECHO OFF
  >>build.bat ECHO CALL "%~dp0build.bat" %%*
)
CALL :ParseArgs %* || EXIT /B 1
SET "PARALLEL=1"
IF /I "!CONFIG!"=="clean" SET "PARALLEL=0"
IF /I "!CONFIG!"=="reset" SET "PARALLEL=0"
IF !PARALLEL! EQU 1 (
  CALL :Build Nexus %*
  IF !EXIT_STATUS! NEQ 0 (
    EXIT /B !EXIT_STATUS!
  )
)
CALL :Build WebApi %*
IF !EXIT_STATUS! NEQ 0 (
  EXIT /B !EXIT_STATUS!
)
IF !PARALLEL! EQU 1 (
  SET "BUILD_TEMP=!ROOT!\_build_tmp"
  IF EXIST "!BUILD_TEMP!" RD /S /Q "!BUILD_TEMP!" || GOTO BuildError
  MD "!BUILD_TEMP!" || GOTO BuildError
)
CALL :BuildApp Applications\AdministrationServer %*
CALL :BuildApp Applications\ChartingServer %*
CALL :BuildApp Applications\ComplianceServer %*
CALL :BuildApp Applications\DefinitionsServer %*
CALL :BuildApp Applications\Lollipop %*
CALL :BuildApp Applications\MarketDataRelayServer %*
CALL :BuildApp Applications\MarketDataServer %*
CALL :BuildApp Applications\ReplayMarketDataFeedClient %*
CALL :BuildApp Applications\RiskServer %*
CALL :BuildApp Applications\Scratch %*
CALL :BuildApp Applications\SimulationMarketDataFeedClient %*
CALL :BuildApp Applications\SimulationOrderExecutionServer %*
CALL :BuildApp Applications\Spire %*
CALL :BuildApp Applications\WebPortal\WebApp %*
CALL :BuildApp Applications\WebPortal %*
IF !PARALLEL! EQU 0 (
  IF !EXIT_STATUS! EQU 0 CALL :Build Nexus %*
  EXIT /B !EXIT_STATUS!
)
:WaitLoop
SET "RUNNING=0"
FOR %%F IN ("!BUILD_TEMP!\*.running") DO (
  SET "RUNNING=1"
)
IF !RUNNING! EQU 1 (
  waitfor /T 1 NexusBuildDelay >NUL 2>&1
  GOTO WaitLoop
)
FOR %%F IN ("!BUILD_TEMP!\*.log") DO (
  IF %%~zF GTR 0 (
    ECHO.
    ECHO ============================================================
    ECHO %%~nF
    ECHO ============================================================
    TYPE "%%F"
  )
)
FOR %%F IN ("!BUILD_TEMP!\*.failed") DO (
  SET "EXIT_STATUS=1"
)
RD /S /Q "!BUILD_TEMP!"
EXIT /B !EXIT_STATUS!
ENDLOCAL

:BuildError
EXIT /B 1

:ParseArgs
SET "DEPENDENCIES=!ROOT!\Nexus\Dependencies"
SET "ARGS="
SET "CONFIG="
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
  SET "CONFIG=!ARG!"
)
SHIFT
GOTO ParseArgsLoop
:ParseArgsDone
FOR %%D IN ("!DEPENDENCIES!") DO (
  SET "DEPENDENCIES=%%~fD"
)
EXIT /B 0

:Build
SET "PROJECT=%~1"
IF NOT EXIST "!PROJECT!" (
  MD "!PROJECT!" || (
    SET "EXIT_STATUS=1"
    EXIT /B 1
  )
)
PUSHD "!PROJECT!" || (
  SET "EXIT_STATUS=1"
  EXIT /B 1
)
CALL "!DIRECTORY!!PROJECT!\build.bat" -DD="!DEPENDENCIES!" !ARGS!
IF ERRORLEVEL 1 SET "EXIT_STATUS=1"
POPD
EXIT /B 0

:BuildApp
IF !PARALLEL! EQU 0 (
  CALL :Build %*
  EXIT /B 0
)
SET "PROJECT=%~1"
SET "PROJECT_NAME=%~n1"
IF NOT EXIST "!PROJECT!" (
  MD "!PROJECT!" || (
    SET "EXIT_STATUS=1"
    EXIT /B 1
  )
)
>"!BUILD_TEMP!\!PROJECT_NAME!.running" ECHO !PROJECT_NAME! || (
  SET "EXIT_STATUS=1"
  EXIT /B 1
)
START /B cmd /c "PUSHD "!ROOT!\!PROJECT!" && CALL "!DIRECTORY!!PROJECT!\build.bat" -DD="!DEPENDENCIES!" !ARGS! && DEL "!BUILD_TEMP!\!PROJECT_NAME!.running" || (ECHO failed > "!BUILD_TEMP!\!PROJECT_NAME!.failed" & DEL "!BUILD_TEMP!\!PROJECT_NAME!.running")" >"!BUILD_TEMP!\!PROJECT_NAME!.log" 2>&1 || (
  SET "EXIT_STATUS=1"
  DEL "!BUILD_TEMP!\!PROJECT_NAME!.running"
)
EXIT /B 0

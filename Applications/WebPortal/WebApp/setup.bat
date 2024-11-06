@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET ROOT=%cd%
CALL "%~dp0..\..\..\WebApi\setup.bat"
SET DALI_COMMIT="2855c56169ece6e82874ac448d3eaeda29613736"
IF NOT EXIST dali (
  git clone https://www.github.com/spiretrading/dali
  IF !ERRORLEVEL! EQU 0 (
    PUSHD dali
    git checkout "!DALI_COMMIT!"
    POPD
  ) ELSE (
    RD /S /Q dali
    SET EXIT_STATUS=1
  )
)
IF EXIST dali (
  PUSHD dali
  git merge-base --is-ancestor "!DALI_COMMIT!" HEAD
  IF !ERRORLEVEL! NEQ 0 (
    git checkout master
    git pull
    git checkout "!DALI_COMMIT!"
  )
  POPD
)
IF NOT EXIST WebApi (
  MD WebApi
  PUSHD WebApi
  CALL "%~dp0..\..\..\WebApi\configure.bat" -DD="%ROOT%"
  POPD
)
ENDLOCAL

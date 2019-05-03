@ECHO OFF
SETLOCAL
SET ROOT=%cd%
CALL "%~dp0..\..\..\WebApi\setup.bat"
IF NOT EXIST dali (
  git clone https://www.github.com/eidolonsystems/dali
)
SET dali_commit="781c8f9a037ebe44b1da725d0d93d168532bff97"
PUSHD dali
git merge-base --is-ancestor "%dali_commit%" HEAD
IF NOT "%ERRORLEVEL%" == "0" (
  git checkout master
  git pull
  git checkout "%dali_commit%"
)
POPD
IF NOT EXIST WebApi (
  MD WebApi
  PUSHD WebApi
  CALL "%~dp0..\..\..\WebApi\configure.bat" -DD="%ROOT%"
  POPD
)
ENDLOCAL

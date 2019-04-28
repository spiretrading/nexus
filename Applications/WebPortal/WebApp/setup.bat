@ECHO OFF
SETLOCAL
SET ROOT=%cd%
IF NOT EXIST Beam (
  git clone https://www.github.com/eidolonsystems/beam Beam
)
SET beam_commit="83011c27e14b05a59c49298073af48646670f7a5"
PUSHD Beam
git merge-base --is-ancestor "%beam_commit%" HEAD
IF NOT "%ERRORLEVEL%" == "0" (
  git checkout master
  git pull
  git checkout "%beam_commit%"
)
POPD
IF NOT EXIST dali (
  git clone https://www.github.com/eidolonsystems/dali
)
SET dali_commit="ed6605bf4e8662db09d615733d44d7b65e718184"
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

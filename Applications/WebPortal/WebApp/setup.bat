@ECHO OFF
SETLOCAL
SET ROOT=%cd%
IF NOT EXIST Beam (
  git clone https://www.github.com/eidolonsystems/beam Beam
)
SET beam_commit="937ac6ef0fbc0fb561f71a31dfe0bbc039bcf093"
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
SET dali_commit="159ca71eb0885febd108b621d75cc09471163e38"
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

@ECHO OFF
SETLOCAL
IF NOT EXIST Beam (
  git clone https://www.github.com/eidolonsystems/beam Beam
)
SET beam_commit="2d851f381b3ad7bce807fca2ad50a204f44ff3ca"
PUSHD Beam
git merge-base --is-ancestor "%beam_commit%" HEAD
IF NOT "%ERRORLEVEL%" == "0" (
  git checkout master
  git pull
  git checkout "%beam_commit%"
)
POPD
ENDLOCAL

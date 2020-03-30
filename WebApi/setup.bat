@ECHO OFF
SETLOCAL
IF NOT EXIST Beam (
  git clone https://www.github.com/spiretrading/beam Beam
)
SET beam_commit="404ff39690eda6314dc0bc4d41954d32a3a3d5f1"
PUSHD Beam
git merge-base --is-ancestor "%beam_commit%" HEAD
IF NOT "%ERRORLEVEL%" == "0" (
  git checkout master
  git pull
  git checkout "%beam_commit%"
)
POPD
ENDLOCAL

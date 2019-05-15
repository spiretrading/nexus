@ECHO OFF
SETLOCAL
IF NOT EXIST Beam (
  git clone https://www.github.com/eidolonsystems/beam Beam
)
SET beam_commit="5a5f7879ec20be618405ce43cce42f952eda6637"
PUSHD Beam
git merge-base --is-ancestor "%beam_commit%" HEAD
IF NOT "%ERRORLEVEL%" == "0" (
  git checkout master
  git pull
  git checkout "%beam_commit%"
)
POPD
ENDLOCAL

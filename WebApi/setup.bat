@ECHO OFF
SETLOCAL
IF NOT EXIST Beam (
  git clone https://www.github.com/spiretrading/beam Beam
)
SET beam_commit="afe2841ee216d75ea3686ccb0e46d064f0f1a1ab"
PUSHD Beam
git merge-base --is-ancestor "%beam_commit%" HEAD
IF NOT "%ERRORLEVEL%" == "0" (
  git checkout master
  git pull
  git checkout "%beam_commit%"
)
POPD
ENDLOCAL

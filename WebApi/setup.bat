@ECHO OFF
SETLOCAL
IF NOT EXIST Beam (
  git clone https://www.github.com/eidolonsystems/beam Beam
)
SET beam_commit="c5f2491aaf245d7cad90cd6f05c1294f9906bd1d"
PUSHD Beam
git merge-base --is-ancestor "%beam_commit%" HEAD
IF NOT "%ERRORLEVEL%" == "0" (
  git checkout master
  git pull
  git checkout "%beam_commit%"
)
POPD
ENDLOCAL

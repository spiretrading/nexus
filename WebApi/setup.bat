@ECHO OFF
SETLOCAL
IF NOT EXIST Beam (
  git clone https://www.github.com/eidolonsystems/beam Beam
)
SET beam_commit="405667ca2e60899886f0978ea6610e4ded263eea"
PUSHD Beam
git merge-base --is-ancestor "%beam_commit%" HEAD
IF NOT "%ERRORLEVEL%" == "0" (
  git checkout master
  git pull
  git checkout "%beam_commit%"
)
POPD
ENDLOCAL

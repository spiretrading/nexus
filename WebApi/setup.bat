@ECHO OFF
SETLOCAL
IF NOT EXIST Beam (
  git clone https://www.github.com/spiretrading/beam Beam
)
SET beam_commit="cfb05f2182b9cfb7314e9f164b2ff58611aee9cd"
PUSHD Beam
git merge-base --is-ancestor "%beam_commit%" HEAD
IF NOT "%ERRORLEVEL%" == "0" (
  git checkout master
  git pull
  git checkout "%beam_commit%"
)
POPD
ENDLOCAL

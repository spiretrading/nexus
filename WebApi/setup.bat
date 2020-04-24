@ECHO OFF
SETLOCAL
IF NOT EXIST Beam (
  git clone https://www.github.com/spiretrading/beam Beam
)
SET beam_commit="9e2b013020de342e10d40018a47ef350a8dd57a9"
PUSHD Beam
git merge-base --is-ancestor "%beam_commit%" HEAD
IF NOT "%ERRORLEVEL%" == "0" (
  git checkout master
  git pull
  git checkout "%beam_commit%"
)
POPD
ENDLOCAL

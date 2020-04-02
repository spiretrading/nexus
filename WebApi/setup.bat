@ECHO OFF
SETLOCAL
IF NOT EXIST Beam (
  git clone https://www.github.com/spiretrading/beam Beam
)
SET beam_commit="e7e3dcae29185c3be4bbd1e0c68d4ef6b675cfbf"
PUSHD Beam
git merge-base --is-ancestor "%beam_commit%" HEAD
IF NOT "%ERRORLEVEL%" == "0" (
  git checkout master
  git pull
  git checkout "%beam_commit%"
)
POPD
ENDLOCAL

@ECHO OFF
SETLOCAL
IF NOT EXIST Beam (
  git clone https://www.github.com/eidolonsystems/beam Beam
)
SET beam_commit="450c0cbc3fbb996b3bb94d11249f5c07e742b1c6"
PUSHD Beam
git merge-base --is-ancestor "%beam_commit%" HEAD
IF NOT "%ERRORLEVEL%" == "0" (
  git checkout master
  git pull
  git checkout "%beam_commit%"
)
POPD
ENDLOCAL

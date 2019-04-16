@ECHO OFF
SETLOCAL
SET ROOT="%cd%"
IF NOT EXIST Beam (
  git clone https://www.github.com/eidolonsystems/beam Beam
)
IF EXIST Beam (
  SET beam_commit="e82694fca1ec7336012998c5f0dbfe0468199680"
  PUSHD Beam
  FOR /f "usebackq tokens=*" %%a IN (`git log -1 ^| head -1 ^| awk "{ print $2 }"`) DO SET commit=%%a
  IF NOT "%commit%" == "%beam_commit%" (
    git checkout master
    git pull
    git checkout %beam_commit%
  )
  POPD
  CALL Beam\Beam\setup.bat
)
ENDLOCAL

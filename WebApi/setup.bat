@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET BEAM_COMMIT="435256c05e7058619103cf281d62e7e8e22ab025"
IF NOT EXIST Beam (
  git clone https://www.github.com/spiretrading/beam Beam
  IF !ERRORLEVEL! EQU 0 (
    PUSHD Beam
    git checkout "!BEAM_COMMIT!"
    POPD
  ) ELSE (
    RD /S /Q Beam
    SET EXIT_STATUS=1
  )
)
IF EXIST Beam (
  PUSHD Beam
  git merge-base --is-ancestor "!BEAM_COMMIT!" HEAD
  IF !ERRORLEVEL! NEQ 0 (
    git checkout master
    git pull
    git checkout "!BEAM_COMMIT!"
  )
  POPD
)
ENDLOCAL

@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET BEAM_COMMIT="e3929a07cc485cd1fff6ab8743a20b6ed6057f26"
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

@ECHO OFF
SETLOCAL
IF [%1] == [] (
  SET config=Release
) ELSE (
  SET config="%1"
)
IF "%1" == "clean" (
  git clean -fxd -e *Dependencies* -e *WebApp*
) ELSE (
  cmake --build . --target INSTALL --config %config%
)
ENDLOCAL

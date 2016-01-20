SETLOCAL
IF [%1] == [] (
  SET config=Debug
) ELSE (
  SET config="%1"
)
cmake --build %~dp0 --target INSTALL --config %config%
ENDLOCAL

SETLOCAL
IF [%1] == [] (
  SET config=Release
) ELSE (
  SET config="%1"
)
cmake --build . --target INSTALL --config %config%
ENDLOCAL

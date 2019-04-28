@ECHO OFF
SETLOCAL
SET ROOT=%cd%
SET UPDATE_NODE=
SET UPDATE_BUILD=
IF "%1" == "clean" (
  IF EXIST library (
    RMDIR /s /q library
  )
  IF EXIST node_modules\mod_time.txt (
    DEL node_modules\mod_time.txt
  )
  EXIT /B
)
IF "%1" == "reset" (
  IF EXIST library (
    RMDIR /s /q library
  )
  IF EXIST node_modules (
    RMDIR /s /q node_modules
  )
  IF EXIST package-lock.json (
    DEL package-lock.json
  )
  IF NOT "%~dp0" == "%ROOT%\" (
    DEL package.json >NUL 2>&1
    DEL tsconfig.json >NUL 2>&1
  )
  EXIT /B
)
IF NOT "%~dp0" == "%ROOT%\" (
  COPY /Y "%~dp0package.json" . >NUL
  COPY /Y "%~dp0tsconfig.json" . >NUL
)
SET BEAM_PATH=Dependencies\Beam\WebApi
PUSHD %BEAM_PATH%
CALL build.bat %*
POPD
IF NOT EXIST node_modules (
  SET UPDATE_NODE=1
) ELSE (
  PUSHD node_modules
  IF NOT EXIST mod_time.txt (
    SET UPDATE_NODE=1
  ) ELSE (
    FOR /F %%i IN (
      'ls -l --time-style=full-iso "%~dp0package.json" ^| awk "{print $6 $7}"') DO (
      FOR /F %%j IN (
        'ls -l --time-style=full-iso mod_time.txt ^| awk "{print $6 $7}"') DO (
        IF "%%i" GEQ "%%j" (
          SET UPDATE_NODE=1
        )
      )
    )
  )
  POPD
)
IF "%UPDATE_NODE%" == "1" (
  SET UPDATE_BUILD=1
  CALL npm install
)
IF NOT EXIST library (
  SET UPDATE_BUILD=1
) ELSE (
  FOR /F %%i IN (
    'DIR source /s/b/a-d ^| tr "\\" "/" ^| xargs ls -l --time-style=full-iso ^| awk "{print $6 $7}" ^| sort /R ^| head -1') DO (
    FOR /F %%j IN (
      'DIR library /s/b/a-d ^| tr "\\" "/" ^| xargs ls -l --time-style=full-iso ^| awk "{print $6 $7}" ^| sort /R ^| head -1') DO (
      IF "%%i" GEQ "%%j" (
        SET UPDATE_BUILD=1
      )
    )
  )
)
IF NOT EXIST node_modules\mod_time.txt (
  SET UPDATE_BUILD=1
) ELSE (
  FOR /F %%i IN (
    'ls -l --time-style=full-iso "%~dp0tsconfig.json" "%BEAM_PATH%\node_modules\mod_time.txt" ^| awk "{print $6 $7}"') DO (
    FOR /F %%j IN (
      'ls -l --time-style=full-iso node_modules\mod_time.txt ^| awk "{print $6 $7}"') DO (
      IF "%%i" GEQ "%%j" (
        SET UPDATE_BUILD=1
      )
    )
  )
)
IF "%UPDATE_BUILD%" == "1" (
  IF EXIST library (
    RMDIR /s /q library
  )
  CALL npm run build
  IF NOT "%ERRORLEVEL%" == "0" (
    ECHO "timestamp" > node_modules\mod_time.txt
  )
)
ENDLOCAL

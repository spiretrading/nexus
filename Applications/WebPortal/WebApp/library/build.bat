@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET ROOT=%cd%
SET UPDATE_NODE=
SET UPDATE_BUILD=
IF "%1" == "clean" (
  IF EXIST library (
    RMDIR /q /s library
  )
  IF EXIST mod_time.txt (
    DEL mod_time.txt
  )
  EXIT /B
)
IF "%1" == "reset" (
  IF EXIST library (
    RMDIR /q /s library
  )
  IF EXIST mod_time.txt (
    DEL mod_time.txt
  )
  IF EXIST node_modules (
    RMDIR /q /s node_modules
  )
  IF EXIST package-lock.json (
    DEL package-lock.json
  )
  IF NOT "%~dp0" == "!ROOT!\" (
    DEL package.json >NUL 2>&1
    DEL tsconfig.json >NUL 2>&1
  )
  EXIT /B
)
IF NOT "%~dp0" == "!ROOT!\" (
  COPY /Y "%~dp0package.json" . >NUL
  COPY /Y "%~dp0tsconfig.json" . >NUL
)
SET DALI_PATH=Dependencies\dali
SET NEXUS_PATH=Dependencies\WebApi
PUSHD !DALI_PATH!
CALL build.bat %*
POPD
PUSHD !NEXUS_PATH!
CALL build.bat %*
POPD
IF NOT EXIST node_modules (
  SET UPDATE_NODE=1
) ELSE (
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
)
IF "!UPDATE_NODE!" == "1" (
  SET UPDATE_BUILD=1
  CALL npm install
)
IF NOT EXIST library (
  SET UPDATE_BUILD=1
) ELSE (
  SET MAX_SOURCE_TIMESTAMP=
  SET MAX_TARGET_TIMESTAMP=
  SET CHUNK=100
  FOR /F %%i IN ('DIR source /s/b/a-d ^| wc -l') DO SET LINE_COUNT=%%i
  FOR /L %%i IN (0,!CHUNK!,!LINE_COUNT!) DO (
    FOR /F %%j IN (
        'DIR source /s/b/a-d ^| tail -n +%%i 2^>NUL ^| head -!CHUNK! ^| tr "\\" "/" ^| xargs ls -l --time-style=full-iso ^| awk "{print $6 $7}" ^| sort /R ^| head -1') DO (
      IF %%j GTR !MAX_SOURCE_TIMESTAMP! (
        SET MAX_SOURCE_TIMESTAMP=%%j
      )
    )
  )
  FOR /F %%i IN ('DIR library /s/b/a-d ^| wc -l') DO SET LINE_COUNT=%%i
  FOR /L %%i IN (0,!CHUNK!,!LINE_COUNT!) DO (
    FOR /F %%j IN (
        'DIR library /s/b/a-d ^| tail -n +%%i 2^>NUL ^| head -!CHUNK! ^| tr "\\" "/" ^| xargs ls -l --time-style=full-iso ^| awk "{print $6 $7}" ^| sort /R ^| head -1') DO (
      IF %%j GTR !MAX_TARGET_TIMESTAMP! (
        SET MAX_TARGET_TIMESTAMP=%%j
      )
    )
  )
  IF !MAX_SOURCE_TIMESTAMP! GEQ !MAX_TARGET_TIMESTAMP! (
    SET UPDATE_BUILD=1
  )
)
IF NOT EXIST mod_time.txt (
  SET UPDATE_BUILD=1
) ELSE (
  FOR /F %%i IN (
      'ls -l --time-style=full-iso "%~dp0tsconfig.json" "!NEXUS_PATH!\mod_time.txt" "!DALI_PATH!\mod_time.txt" ^| awk "{print $6 $7}"') DO (
    FOR /F %%j IN (
        'ls -l --time-style=full-iso mod_time.txt ^| awk "{print $6 $7}"') DO (
      IF "%%i" GEQ "%%j" (
        SET UPDATE_BUILD=1
      )
    )
  )
)
IF "!UPDATE_BUILD!" == "1" (
  IF EXIST library (
    RMDIR /q /s library
  )
  CALL npm run build
  ECHO "timestamp" > mod_time.txt
)
ENDLOCAL

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
    DEL webpack.config.js >NUL 2>&1
  )
  EXIT /B
)
IF NOT "%~dp0" == "%ROOT%\" (
  COPY /Y "%~dp0package.json" . >NUL
  COPY /Y "%~dp0tsconfig.json" . >NUL
  COPY /Y "%~dp0webpack.config.js" . >NUL
)
SET WEB_PORTAL_PATH=Dependencies\library
PUSHD %WEB_PORTAL_PATH%
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
      'ls -l --time-style=full-iso ..\package.json ^| awk "{print $6 $7}"') DO (
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
IF NOT EXIST application (
  SET UPDATE_BUILD=1
) ELSE (
  FOR /F %%i IN (
    'DIR source /s/b/a-d ^| tr "\\" "/" ^| xargs ls -l --time-style=full-iso ^| awk "{print $6 $7}" ^| sort /R ^| head -1') DO (
    FOR /F %%j IN (
      'DIR application /s/b/a-d ^| tr "\\" "/" ^| xargs ls -l --time-style=full-iso ^| awk "{print $6 $7}" ^| sort /R ^| head -1') DO (
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
    'ls -l --time-style=full-iso tsconfig.json %WEB_PORTAL_PATH%\node_modules\mod_time.txt ^| awk "{print $6 $7}"') DO (
    FOR /F %%j IN (
      'ls -l --time-style=full-iso node_modules\mod_time.txt ^| awk "{print $6 $7}"') DO (
      IF "%%i" GEQ "%%j" (
        SET UPDATE_BUILD=1
      )
    )
  )
)
IF "%UPDATE_BUILD%" == "1" (
  IF EXIST application (
    RMDIR /s /q application
  )
  node node_modules\webpack\bin\webpack.js
  ECHO "timestamp" > node_modules\mod_time.txt
  robocopy "%~dp0../../resources" application /E
  COPY "%~dp0source/index.html" application
)
ENDLOCAL

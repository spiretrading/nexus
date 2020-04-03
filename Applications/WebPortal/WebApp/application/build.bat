@ECHO OFF
SETLOCAL
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
IF NOT EXIST mod_time.txt (
  SET UPDATE_BUILD=1
) ELSE (
  FOR /F %%i IN (
    'ls -l --time-style=full-iso "%~dp0tsconfig.json" "%~dp0webpack.config.js" "%WEB_PORTAL_PATH%\mod_time.txt" ^| awk "{print $6 $7}"') DO (
    FOR /F %%j IN (
      'ls -l --time-style=full-iso mod_time.txt ^| awk "{print $6 $7}"') DO (
      IF "%%i" GEQ "%%j" (
        SET UPDATE_BUILD=1
      )
    )
  )
)
IF "%UPDATE_BUILD%" == "1" (
  IF EXIST application (
    DEL /s application\*
  )
  node node_modules\webpack\bin\webpack.js
  ECHO "timestamp" > mod_time.txt
  IF EXIST application (
    robocopy "%~dp0..\resources" application\resources /E > NUL
    COPY "%~dp0source\index.html" application\index.html > NUL
    IF EXIST ..\..\Application (
      IF NOT EXIST ..\..\Application\web_app (
        PUSHD ..\..\Application
        MD web_app
        POPD
      )
      robocopy application ..\..\Application\web_app /E > NUL
    )
  )
)
ENDLOCAL

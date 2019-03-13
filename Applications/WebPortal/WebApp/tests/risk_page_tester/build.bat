@ECHO OFF
SETLOCAL
SET UPDATE_NODE=
SET UPDATE_BUILD=
SET PROD_ENV=
IF NOT "%1" == "Debug" (
  SET PROD_ENV=1
)
PUSHD %~dp0
IF "%1" == "clean" (
  IF EXIST application (
    rmdir /s /q application
  )
  IF EXIST node_modules\mod_time.txt (
    del node_modules\mod_time.txt
  )
  EXIT /B
)
IF "%1" == "reset" (
  IF EXIST application (
    rmdir /s /q application
  )
  IF EXIST node_modules (
    rmdir /s /q node_modules
  )
  IF EXIST package-lock.json (
    del package-lock.json
  )
  EXIT /B
)
SET WEB_PORTAL_PATH=..\..\library
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
    'dir source /s/b/a-d ^| tr "\\" "/" ^| xargs ls -l --time-style=full-iso ^| awk "{print $6 $7}" ^| sort /R ^| head -1') DO (
    FOR /F %%j IN (
      'dir application /s/b/a-d ^| tr "\\" "/" ^| xargs ls -l --time-style=full-iso ^| awk "{print $6 $7}" ^| sort /R ^| head -1') DO (
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
    rm -rf application
  )
  node .\node_modules\webpack\bin\webpack.js
  echo "timestamp" > node_modules\mod_time.txt
  cp -r ../../resources application
  cp -r source/index.html application
)
POPD
ENDLOCAL

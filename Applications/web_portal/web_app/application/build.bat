ECHO OFF
SETLOCAL
SET UPDATE_NODE=
SET UPDATE_BUILD=
SET PROD_ENV=
PUSHD %~dp0
SET BEAM_PATH=..\..\..\..\..\Beam\web_api
SET NEXUS_PATH=..\..\..\..\web_api
SET WEB_PORTAL_PATH=..\library
IF NOT "%1" == "Debug" (
  SET PROD_ENV=1
)
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
    IF "%UPDATE_NODE%" == "" (
      IF NOT EXIST ..\%BEAM_PATH%\library (
        SET UPDATE_NODE=1
      ) ELSE (
        FOR /F %%i IN (
          'dir ..\%BEAM_PATH%\source /s/b/a-d ^| tr "\\" "/" ^| xargs ls -l --time-style=full-iso ^| awk "{print $6 $7}" ^| sort /R ^| head -1') DO (
          FOR /F %%j IN (
            'ls -l --time-style=full-iso mod_time.txt ^| awk "{print $6 $7}"') DO (
            IF "%%i" GEQ "%%j" (
              SET UPDATE_NODE=1
            )
          )
        )
      )
    )
    IF "%UPDATE_NODE%" == "" (
      IF NOT EXIST ..\%NEXUS_PATH%\library (
        SET UPDATE_NODE=1
      ) ELSE (
        FOR /F %%i IN (
          'dir ..\%NEXUS_PATH%\source /s/b/a-d ^| tr "\\" "/" ^| xargs ls -l --time-style=full-iso ^| awk "{print $6 $7}" ^| sort /R ^| head -1') DO (
          FOR /F %%j IN (
            'ls -l --time-style=full-iso mod_time.txt ^| awk "{print $6 $7}"') DO (
            IF "%%i" GEQ "%%j" (
              SET UPDATE_NODE=1
            )
          )
        )
      )
    )
    IF "%UPDATE_NODE%" == "" (
      IF NOT EXIST ..\%WEB_PORTAL_PATH%\library (
        SET UPDATE_NODE=1
      ) ELSE (
        FOR /F %%i IN (
          'dir ..\%WEB_PORTAL_PATH%\source /s/b/a-d ^| tr "\\" "/" ^| xargs ls -l --time-style=full-iso ^| awk "{print $6 $7}" ^| sort /R ^| head -1') DO (
          FOR /F %%j IN (
            'ls -l --time-style=full-iso mod_time.txt ^| awk "{print $6 $7}"') DO (
            IF "%%i" GEQ "%%j" (
              SET UPDATE_NODE=1
            )
          )
        )
      )
    )
  )
  POPD
)
IF "%UPDATE_NODE%" == "1" (
  SET UPDATE_BUILD=1
  PUSHD %BEAM_PATH%
  CALL build.bat
  POPD
  PUSHD %NEXUS_PATH%
  CALL build.bat
  POPD
  PUSHD %WEB_PORTAL_PATH%
  CALL build.bat
  POPD
  CALL npm install
  PUSHD node_modules
  IF EXIST beam (
    rm -rf beam
  )
  cp -r ..\%BEAM_PATH%\library\* .
  IF EXIST @types\beam (
    rm -rf @types\beam
  )
  mkdir @types\beam
  cp -r ..\%BEAM_PATH%\library\beam\library\beam\* @types\beam
  IF EXIST nexus (
    rm -rf nexus
  )
  IF EXIST nexus (
    rm -rf nexus
  )
  cp -r ..\%NEXUS_PATH%\library\* .
  IF EXIST @types\nexus (
    rm -rf @types\nexus
  )
  mkdir @types\nexus
  cp -r ..\%NEXUS_PATH%\library\nexus\library\nexus\* @types\nexus
  IF EXIST web_portal (
    rm -rf web_portal
  )
  cp -r ..\%WEB_PORTAL_PATH%\library\* .
  IF EXIST @types\web_portal (
    rm -rf @types\web_portal
  )
  mkdir @types\web_portal
  cp -r ..\%WEB_PORTAL_PATH%\library\web_portal\library\web_portal\* @types\web_portal
  echo "timestamp" > mod_time.txt
  POPD
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
IF "%UPDATE_BUILD%" == "1" (
  IF EXIST application (
    rm -rf application
  )
  node .\node_modules\webpack\bin\webpack.js
  cp -r ../resources application
  cp -r source/index.html application
)
POPD
ENDLOCAL

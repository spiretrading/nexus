@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET DIRECTORY=%~dp0
SET ROOT=%cd%
:begin_args
SET ARG=%~1
IF "!IS_DEPENDENCY!" == "1" (
  SET DEPENDENCIES=!ARG!
  SET IS_DEPENDENCY=
  SHIFT
  GOTO begin_args
) ELSE IF NOT "!ARG!" == "" (
  IF "!ARG:~0,3!" == "-DD" (
    SET IS_DEPENDENCY=1
  ) ELSE (
    SET CONFIG=!ARG!
  )
  SHIFT
  GOTO begin_args
)
IF "!CONFIG!" == "" (
  SET CONFIG=Release
)
IF "!CONFIG!" == "clean" (
  IF EXIST application (
    RMDIR /q /s application
  )
  IF EXIST mod_time.txt (
    DEL mod_time.txt
  )
  EXIT /B
)
IF "!CONFIG!" == "reset" (
  IF EXIST application (
    RMDIR /q /s application
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
  IF NOT "!DIRECTORY!" == "!ROOT!\" (
    DEL package.json >NUL 2>&1
    DEL tsconfig.json >NUL 2>&1
    DEL webpack.config.js >NUL 2>&1
  )
  EXIT /B
)
IF NOT "!DIRECTORY!" == "!ROOT!\" (
  COPY /Y "!DIRECTORY!package.json" . >NUL
  COPY /Y "!DIRECTORY!tsconfig.json" . >NUL
  COPY /Y "!DIRECTORY!webpack.config.js" . >NUL
)
IF NOT "!DEPENDENCIES!" == "" (
  CALL "!DIRECTORY!configure.bat" -DD=!DEPENDENCIES!
) ELSE (
  CALL "!DIRECTORY!configure.bat"
)
SET WEB_PORTAL_PATH=Dependencies\library
PUSHD !WEB_PORTAL_PATH!
CALL build.bat %*
POPD
SET UPDATE_NODE=
IF NOT EXIST node_modules (
  SET UPDATE_NODE=1
) ELSE (
  IF NOT EXIST mod_time.txt (
    SET UPDATE_NODE=1
  ) ELSE (
    FOR /F %%i IN (
        'ls -l --time-style=full-iso "!DIRECTORY!package.json" ^| awk "{print $6 $7}"') DO (
      FOR /F %%j IN (
          'ls -l --time-style=full-iso mod_time.txt ^| awk "{print $6 $7}"') DO (
        IF "%%i" GEQ "%%j" (
          SET UPDATE_NODE=1
        )
      )
    )
  )
)
SET UPDATE_BUILD=
IF "!UPDATE_NODE!" == "1" (
  SET UPDATE_BUILD=1
  CALL npm install --no-package-lock
)
IF NOT EXIST application (
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
  FOR /F %%i IN ('DIR application /s/b/a-d ^| wc -l') DO SET LINE_COUNT=%%i
  FOR /L %%i IN (0,!CHUNK!,!LINE_COUNT!) DO (
    FOR /F %%j IN (
        'DIR application /s/b/a-d ^| tail -n +%%i 2^>NUL ^| head -!CHUNK! ^| tr "\\" "/" ^| xargs ls -l --time-style=full-iso ^| awk "{print $6 $7}" ^| sort /R ^| head -1') DO (
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
      'ls -l --time-style=full-iso "!DIRECTORY!tsconfig.json" "!DIRECTORY!webpack.config.js" "!WEB_PORTAL_PATH!\mod_time.txt" ^| awk "{print $6 $7}"') DO (
    FOR /F %%j IN (
        'ls -l --time-style=full-iso mod_time.txt ^| awk "{print $6 $7}"') DO (
      IF "%%i" GEQ "%%j" (
        SET UPDATE_BUILD=1
      )
    )
  )
)
IF "!UPDATE_BUILD!" == "1" (
  IF EXIST application (
    RMDIR /s /q application
  )
  IF "!CONFIG!" == "Release" (
    SET PROD_ENV=1
  )
  node node_modules\webpack\bin\webpack.js
  IF "!CONFIG!" == "Release" (
    SET PROD_ENV=
  )
  ECHO "timestamp" > mod_time.txt
  IF EXIST application (
    robocopy "!DIRECTORY!..\..\resources" application\resources /E > NUL
    COPY "!DIRECTORY!source\index.html" application\index.html > NUL
  )
)
ENDLOCAL

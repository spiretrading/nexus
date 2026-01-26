@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET "DIRECTORY=%~dp0"
SET "ROOT=%cd%"
SET "WEB_PORTAL_PATH=Dependencies\library"
CALL :ParseArgs %*
IF /I "!CONFIG!"=="clean" (
  CALL :CleanBuild "clean"
  EXIT /B !ERRORLEVEL!
)
IF /I "!CONFIG!"=="reset" (
  CALL :CleanBuild "reset"
  EXIT /B !ERRORLEVEL!
)
CALL :Configure || EXIT /B 1
CALL :BuildDependencies || EXIT /B 1
CALL :CheckNodeModules || EXIT /B 1
CALL :CheckBuild || EXIT /B 1
CALL :RunBuild
EXIT /B !ERRORLEVEL!
ENDLOCAL

:ParseArgs
SET "DEPENDENCIES="
SET "IS_DEPENDENCY="
SET "IS_DIRECTORY="
SET "CONFIG="
:ParseArgsLoop
SET "ARG=%~1"
IF "!IS_DEPENDENCY!"=="1" (
  SET "DEPENDENCIES=!ARG!"
  SET "IS_DEPENDENCY="
  SHIFT
  GOTO ParseArgsLoop
) ELSE IF "!IS_DIRECTORY!"=="1" (
  SET "DIRECTORY=!ARG!"
  SET "IS_DIRECTORY="
  SHIFT
  GOTO ParseArgsLoop
) ELSE IF NOT "!ARG!"=="" (
  IF "!ARG:~0,4!"=="-DD=" (
    SET "DEPENDENCIES=!ARG:~4!"
  ) ELSE IF "!ARG!"=="-DD" (
    SET "IS_DEPENDENCY=1"
  ) ELSE IF "!ARG:~0,3!"=="-D=" (
    SET "DIRECTORY=!ARG:~3!"
  ) ELSE IF "!ARG!"=="-D" (
    SET "IS_DIRECTORY=1"
  ) ELSE (
    SET "CONFIG=!ARG!"
  )
  SHIFT
  GOTO ParseArgsLoop
)
IF "!CONFIG!"=="" (
  SET "CONFIG=Release"
)
EXIT /B 0

:CleanBuild
RD /S /Q application 2>NUL
DEL mod_time.txt >NUL 2>&1
IF "%~1"=="reset" (
  RD /S /Q Dependencies 2>NUL
  RD /S /Q node_modules 2>NUL
  IF NOT "!DIRECTORY!"=="!ROOT!\" (
    DEL package.json >NUL 2>&1
    DEL tsconfig.json >NUL 2>&1
    DEL webpack.config.js >NUL 2>&1
  )
)
EXIT /B 0

:Configure
IF NOT "!DIRECTORY!"=="!ROOT!\" (
  COPY /Y "!DIRECTORY!package.json" . >NUL || EXIT /B 1
  COPY /Y "!DIRECTORY!tsconfig.json" . >NUL || EXIT /B 1
  COPY /Y "!DIRECTORY!webpack.config.js" . >NUL || EXIT /B 1
)
IF NOT "!DEPENDENCIES!"=="" (
  CALL "!DIRECTORY!configure.bat" -DD="!DEPENDENCIES!"
) ELSE (
  CALL "!DIRECTORY!configure.bat"
)
EXIT /B !ERRORLEVEL!

:BuildDependencies
PUSHD !WEB_PORTAL_PATH!
CALL build.bat %* || (POPD & EXIT /B 1)
POPD
EXIT /B 0

:CheckNodeModules
SET "UPDATE_NODE="
IF NOT EXIST node_modules (
  SET "UPDATE_NODE=1"
) ELSE IF NOT EXIST mod_time.txt (
  SET "UPDATE_NODE=1"
) ELSE (
  SET CHECK_PKG_COMMAND=powershell -NoProfile -Command "& {" ^
    "$mod = (Get-Item 'mod_time.txt').LastWriteTime.Ticks;" ^
    "$pkg = (Get-Item '!DIRECTORY!package.json').LastWriteTime.Ticks;" ^
    "if ($pkg -gt $mod) { 'YES' } else { 'NO' }" ^
  "}"
  FOR /F "delims=" %%r IN ('CALL !CHECK_PKG_COMMAND!') DO (
    SET "NEEDS_NODE_UPDATE=%%r"
  )
  IF "!NEEDS_NODE_UPDATE!"=="YES" (
    SET "UPDATE_NODE=1"
  )
)
IF "!UPDATE_NODE!"=="1" (
  SET "UPDATE_BUILD=1"
  CALL npm install || EXIT /B 1
)
EXIT /B 0

:CheckBuild
IF NOT EXIST application (
  SET "UPDATE_BUILD=1"
) ELSE IF NOT EXIST mod_time.txt (
  SET "UPDATE_BUILD=1"
) ELSE (
  SET CHECK_BUILD_COMMAND=powershell -NoProfile -Command "& {" ^
    "$mod = (Get-Item 'mod_time.txt').LastWriteTime.Ticks;" ^
    "$tsconfig = Get-Item '!DIRECTORY!tsconfig.json';" ^
    "$webpack = Get-Item '!DIRECTORY!webpack.config.js';" ^
    "$webPortalMod = Get-Item '!WEB_PORTAL_PATH!\mod_time.txt';" ^
    "$sourceFiles = Get-ChildItem -Path '!DIRECTORY!source'" ^
    "  -Recurse -File -ErrorAction SilentlyContinue;" ^
    "$files = @($tsconfig, $webpack, $webPortalMod) + $sourceFiles;" ^
    "if ($files) {" ^
    "  $newest = $files | Sort-Object LastWriteTime -Descending |" ^
    "    Select-Object -First 1;" ^
    "  if ($newest.LastWriteTime.Ticks -gt $mod) {" ^
    "    'Result: YES'" ^
    "  } else {" ^
    "    'Result: NO'" ^
    "  }" ^
    "} else {" ^
    "  'Result: NO'" ^
    "}" ^
  "}"
  FOR /F "tokens=2" %%r IN ('CALL !CHECK_BUILD_COMMAND!') DO (
    SET "NEEDS_BUILD=%%r"
  )
  IF "!NEEDS_BUILD!"=="YES" (
    SET "UPDATE_BUILD=1"
  )
)
EXIT /B 0

:RunBuild
IF "!UPDATE_BUILD!"=="1" (
  IF EXIST application (
    DEL /S /Q /F application\* >NUL
  )
  IF /I "!CONFIG!"=="Release" (
    SET "PROD_ENV=1"
  )
  node node_modules\webpack\bin\webpack.js || EXIT /B 1
  IF /I "!CONFIG!"=="Release" (
    SET "PROD_ENV="
  )
  ECHO. > mod_time.txt
  IF EXIST application (
    robocopy "%~dp0..\resources" application\resources /E >NUL
    COPY "!DIRECTORY!source\index.html" application\index.html >NUL
    IF EXIST ..\..\Application (
      IF NOT EXIST ..\..\Application\web_app (
        PUSHD ..\..\Application
        MD web_app
        POPD
      )
      robocopy application ..\..\Application\web_app /E >NUL
    )
  )
)
EXIT /B 0

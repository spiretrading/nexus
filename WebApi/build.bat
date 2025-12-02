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
IF "!CONFIG!" == "clean" (
  IF EXIST library (
    RMDIR /q /s library
  )
  IF EXIST mod_time.txt (
    DEL mod_time.txt
  )
  EXIT /B
)
IF "!CONFIG!" == "reset" (
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
  IF NOT "!DIRECTORY!" == "!ROOT!\" (
    DEL package.json >NUL 2>&1
    DEL tsconfig.json >NUL 2>&1
  )
  EXIT /B
)
IF NOT "!DIRECTORY!" == "!ROOT!\" (
  COPY /Y "!DIRECTORY!package.json" . >NUL
  COPY /Y "!DIRECTORY!tsconfig.json" . >NUL
)
IF NOT "!DEPENDENCIES!" == "" (
  CALL "!DIRECTORY!configure.bat" -DD=!DEPENDENCIES!
) ELSE (
  CALL "!DIRECTORY!configure.bat"
)
SET BEAM_PATH=Dependencies\Beam\WebApi
PUSHD !BEAM_PATH!
CALL build.bat %*
POPD
SET UPDATE_NODE=
IF NOT EXIST node_modules (
  SET UPDATE_NODE=1
) ELSE IF NOT EXIST mod_time.txt (
  SET UPDATE_NODE=1
) ELSE (
  SET CHECK_PKG_COMMAND=powershell -NoProfile -Command "& {" ^
    "$mod = (Get-Item 'mod_time.txt').LastWriteTime.Ticks;" ^
    "$pkg = (Get-Item '!DIRECTORY!package.json').LastWriteTime.Ticks;" ^
    "if ($pkg -gt $mod) { 'YES' } else { 'NO' }" ^
  "}"
  FOR /F "delims=" %%r IN ('CALL !CHECK_PKG_COMMAND!') DO (
    SET NEEDS_NODE_UPDATE=%%r
  )
  IF "!NEEDS_NODE_UPDATE!" == "YES" (
    SET UPDATE_NODE=1
  )
)
SET UPDATE_BUILD=
IF "!UPDATE_NODE!" == "1" (
  SET UPDATE_BUILD=1
  CALL npm install --no-package-lock
)
IF NOT EXIST library (
  SET UPDATE_BUILD=1
) ELSE IF NOT EXIST mod_time.txt (
  SET UPDATE_BUILD=1
) ELSE (
  SET CHECK_BUILD_COMMAND=powershell -NoProfile -Command "& {" ^
    "$mod = (Get-Item 'mod_time.txt').LastWriteTime.Ticks;" ^
    "$tsconfig = Get-Item '!DIRECTORY!tsconfig.json';" ^
    "$beamMod = Get-Item '!BEAM_PATH!\mod_time.txt';" ^
    "$sourceFiles = Get-ChildItem -Path '!DIRECTORY!source'" ^
    "  -Recurse -File -ErrorAction SilentlyContinue;" ^
    "$files = @($tsconfig, $beamMod) + $sourceFiles;" ^
    "if ($files) {" ^
    "  $newest = ($files | Sort-Object LastWriteTime -Descending |" ^
    "    Select-Object -First 1);" ^
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
    SET NEEDS_BUILD=%%r
  )
  IF "!NEEDS_BUILD!" == "YES" (
    SET UPDATE_BUILD=1
  )
)
IF "!UPDATE_BUILD!" == "1" (
  IF EXIST library (
    RMDIR /q /s library
  )
  CALL npm run build
  ECHO. > mod_time.txt
)
ENDLOCAL

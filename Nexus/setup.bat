@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET EXIT_STATUS=0
SET ROOT=%cd%
FOR /F "skip=1" %%H IN ('certutil -hashfile "%~dp0setup.bat" SHA256') DO (
  IF NOT DEFINED SETUP_HASH SET SETUP_HASH=%%H
)
IF EXIST cache_files\nexus.txt (
  SET /P CACHED_HASH=<cache_files\nexus.txt
  IF "!SETUP_HASH!"=="!CACHED_HASH!" EXIT /B 0
)
SET VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
FOR /F "usebackq delims=" %%i IN (` ^
    !VSWHERE! -prerelease -latest -property installationPath`) DO (
  IF EXIST "%%i\Common7\Tools\vsdevcmd.bat" (
    CALL "%%i\Common7\Tools\vsdevcmd.bat" -arch=amd64
  )
)
SET BUILD_BEAM=
SET BEAM_COMMIT="016e7a5123c2cf8b4e40158c66b4d85ca144a9d8"
IF NOT EXIST Beam (
  git clone https://www.github.com/spiretrading/beam Beam
  IF !ERRORLEVEL! EQU 0 (
    SET BUILD_BEAM=1
    PUSHD Beam
    git checkout "!BEAM_COMMIT!"
    POPD
  ) ELSE (
    RD /S /Q Beam
    SET EXIT_STATUS=1
  )
)
IF EXIST Beam (
  PUSHD Beam
  git merge-base --is-ancestor "!BEAM_COMMIT!" HEAD
  IF !ERRORLEVEL! NEQ 0 (
    git checkout master
    git pull
    git checkout "!BEAM_COMMIT!"
    SET BUILD_BEAM=1
  )
  IF !BUILD_BEAM! EQU 1 (
    CALL build.bat Debug -DD="!ROOT!"
    CALL build.bat Release -DD="!ROOT!"
  ) ELSE (
    PUSHD !ROOT!
    CALL Beam\Beam\setup.bat
    POPD
  )
  POPD
)
SET PATH=!PATH!;!ROOT!\Strawberry\perl\site\bin;!ROOT!\Strawberry\perl\bin;!ROOT!\Strawberry\c\bin
IF NOT EXIST qt-5.15.13 (
  git clone --branch v5.15.13-lts-lgpl --depth 1 ^
    https://code.qt.io/qt/qt5.git qt-5.15.13
  IF !ERRORLEVEL! EQU 0 (
    PUSHD qt-5.15.13
    perl init-repository --module-subset=qtbase,qtsvg,qttools,qttranslations
    CALL configure -prefix %cd% -opensource -static -mp -make libs ^
      -make tools -nomake examples -nomake tests -opengl desktop ^
      -no-feature-vulkan -no-icu -qt-freetype -qt-harfbuzz -qt-libpng ^
      -qt-pcre -qt-zlib -confirm-license
    SET CL=/MP
    nmake
    DEL qtbase\lib\cmake\Qt5Core\Qt5CoreConfigExtrasMkspecDir.cmake
    COPY NUL qtbase\lib\cmake\Qt5Core\Qt5CoreConfigExtrasMkspecDir.cmake
    POPD
  ) ELSE (
    RD /S /Q qt-5.15.13
    SET EXIT_STATUS=1
  )
)
CALL :DownloadAndExtract "lua-5.5.0" ^
  "https://www.lua.org/ftp/lua-5.5.0.tar.gz" ^
  "57ccc32bbbd005cab75bcc52444052535af691789dba2b9016d5c50640d68b3d"
IF !BUILD_NEEDED!==1 (
  PUSHD lua-5.5.0\src
  COPY %~dp0\Config\lua.cmake CMakeLists.txt
  cmake .
  cmake --build . --target ALL_BUILD --config Debug
  cmake --build . --target ALL_BUILD --config Release
  POPD
)
CALL :DownloadAndExtract "quickfix-v.1.15.1" ^
  "https://github.com/quickfix/quickfix/archive/49b3508e48f0bbafbab13b68be72250bdd971ac2.zip" ^
  "0bed2ae8359fc807f351fd2d08cec13b472d27943460f1d8f0869ed8cc8c2735"
IF !BUILD_NEEDED!==1 (
  PUSHD quickfix-v.1.15.1
  PUSHD src\C++
  powershell -NoProfile -ExecutionPolicy Bypass -Command "& {" ^
    "$i = 0;" ^
    "(Get-Content 'Utility.h') | ForEach-Object {" ^
    "  $i++;" ^
    "  if($i -eq 105 -or $i -eq 108) {" ^
    "    'template<typename T> using SmartPtr = std::shared_ptr<T>;'" ^
    "  } else {" ^
    "    $_" ^
    "  }" ^
    "} | Set-Content 'Utility.h';" ^
    "@('DOMDocument.h', 'Message.h', 'Message.cpp') | ForEach-Object {" ^
    "  (Get-Content $_) -replace 'std::auto_ptr', 'std::shared_ptr' | " ^
    "    Set-Content $_" ^
    "};" ^
    "(Get-Content 'Message.cpp') -replace 'pGroup\.release\(\)', " ^
    "  'new Group(*pGroup)' | Set-Content 'Message.cpp';" ^
  "}"
  POPD
  cmake .
  cmake --build . --target quickfix --config Debug
  cmake --build . --target quickfix --config Release
  MD "include\quickfix"
  xcopy /s src\C++\* include\quickfix
  POPD
)
CALL :DownloadAndExtract "hat-trie-0.7.0" ^
  "https://github.com/Tessil/hat-trie/archive/refs/tags/v0.7.0.zip" ^
  "8ea5441c06fd5d9de1ec8725bf762025a63f931949b9f49d211ab76a75ced68f"
IF !EXIT_STATUS! NEQ 0 (
  EXIT /B !EXIT_STATUS!
)
IF NOT EXIST cache_files (
  MD cache_files
)
>cache_files\nexus.txt ECHO !SETUP_HASH!
EXIT /B 0

:DownloadAndExtract
SET FOLDER=%~1
SET URL=%~2
SET EXPECTED_HASH=%~3
SET BUILD_NEEDED=0
FOR /F "tokens=* delims=/" %%A IN ("%URL%") DO (
  SET ARCHIVE=%%~nxA
)
IF EXIST !FOLDER! (
  EXIT /B 0
)
IF NOT EXIST !ARCHIVE! (
  curl -fsL -o "!ARCHIVE!" "!URL!"
  IF ERRORLEVEL 1 (
    ECHO Error: Failed to download !ARCHIVE!.
    SET EXIT_STATUS=1
    EXIT /B
  )
)
FOR /F "skip=1 tokens=*" %%H IN ('certutil -hashfile "!ARCHIVE!" SHA256') DO (
  IF NOT DEFINED ACTUAL_HASH SET ACTUAL_HASH=%%H
)
SET ACTUAL_HASH=!ACTUAL_HASH: =!
IF /I NOT "!ACTUAL_HASH!"=="!EXPECTED_HASH!" (
  ECHO Error: SHA256 mismatch for !ARCHIVE!.
  ECHO   Expected: !EXPECTED_HASH!
  ECHO   Actual:   !ACTUAL_HASH!
  DEL /F /Q "!ARCHIVE!"
  SET EXIT_STATUS=1
  SET ACTUAL_HASH=
  EXIT /B
)
SET ACTUAL_HASH=
SET EXTRACT_PATH=_extract_tmp
RD /S /Q "!EXTRACT_PATH!" >NUL 2>NUL
MD "!EXTRACT_PATH!"
tar -xf "!ARCHIVE!" -C "!EXTRACT_PATH!"
IF ERRORLEVEL 1 (
  ECHO Error: Failed to extract !ARCHIVE!.
  SET EXIT_STATUS=1
  EXIT /B
)
SET DETECTED_FOLDER=
FOR %%F IN ("!EXTRACT_PATH!\*") DO (
  IF "!DETECTED_FOLDER!"=="" (
    SET DETECTED_FOLDER=%%F
  ) ELSE (
    SET DETECTED_FOLDER=MULTIPLE
  )
)
FOR /D %%F IN ("!EXTRACT_PATH!\*") DO (
  IF "!DETECTED_FOLDER!"=="" (
    SET DETECTED_FOLDER=%%F
  ) ELSE (
    SET DETECTED_FOLDER=MULTIPLE
  )
)
IF "!DETECTED_FOLDER!"=="MULTIPLE" (
  REN "!EXTRACT_PATH!" "!FOLDER!"
) ELSE IF NOT "!DETECTED_FOLDER!"=="!EXTRACT_PATH!\!FOLDER!" (
  MOVE /Y "!DETECTED_FOLDER!" "!FOLDER!" >NUL
  RD /S /Q "!EXTRACT_PATH!" >NUL 2>NUL
) ELSE (
  MOVE /Y "!EXTRACT_PATH!\!FOLDER!" "!ROOT!" >NUL
  RD /S /Q "!EXTRACT_PATH!" >NUL 2>NUL
)
SET BUILD_NEEDED=1
DEL /F /Q "!ARCHIVE!"
EXIT /B 0

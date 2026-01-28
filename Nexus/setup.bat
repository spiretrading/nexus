@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET "ROOT=%cd%"
CALL :CheckCache "nexus"
IF ERRORLEVEL 1 EXIT /B 0
CALL :SetupVSEnvironment
CALL :AddRepo "Beam" ^
  "https://www.github.com/spiretrading/beam" ^
  "96faba0c3b7dffe659399e959a599ffae765d1a1" ^
  ":BuildBeam"
CALL :InstallRepos || EXIT /B 1
SET "PATH=!PATH!;!ROOT!\Strawberry\perl\site\bin;!ROOT!\Strawberry\perl\bin;!ROOT!\Strawberry\c\bin"
CALL :InstallQt || EXIT /B 1
CALL :AddDependency "lua-5.5.0" ^
  "https://www.lua.org/ftp/lua-5.5.0.tar.gz" ^
  "57ccc32bbbd005cab75bcc52444052535af691789dba2b9016d5c50640d68b3d" ^
  ":BuildLua"
CALL :AddDependency "quickfix-v.1.15.1" ^
  "https://github.com/quickfix/quickfix/archive/49b3508e48f0bbafbab13b68be72250bdd971ac2.zip" ^
  "0bed2ae8359fc807f351fd2d08cec13b472d27943460f1d8f0869ed8cc8c2735" ^
  ":BuildQuickfix"
CALL :AddDependency "hat-trie-0.7.0" ^
  "https://github.com/Tessil/hat-trie/archive/refs/tags/v0.7.0.zip" ^
  "8ea5441c06fd5d9de1ec8725bf762025a63f931949b9f49d211ab76a75ced68f"
CALL :InstallDependencies || EXIT /B 1
CALL :Commit
EXIT /B !ERRORLEVEL!
ENDLOCAL

:BuildBeam
CALL build.bat Debug -DD="!ROOT!" || EXIT /B 1
CALL build.bat Release -DD="!ROOT!" || EXIT /B 1
EXIT /B 0

:InstallQt
IF EXIST "qt-5.15.13" EXIT /B 0
git clone --branch v5.15.13-lts-lgpl --depth 1 ^
  https://code.qt.io/qt/qt5.git qt-5.15.13
IF ERRORLEVEL 1 (
  RD /S /Q "qt-5.15.13" >NUL 2>NUL
  EXIT /B 1
)
PUSHD qt-5.15.13
perl init-repository --module-subset=qtbase,qtsvg,qttools,qttranslations
CALL configure -prefix !cd!\qtbase -opensource -static -mp -make libs ^
  -make tools -nomake examples -nomake tests -opengl desktop ^
  -no-feature-vulkan -no-icu -qt-freetype -qt-harfbuzz -qt-libpng ^
  -qt-pcre -qt-zlib -confirm-license
SET "CL=/MP"
nmake
POPD
EXIT /B 0

:BuildLua
PUSHD src
COPY "%~dp0Config\lua.cmake" CMakeLists.txt || (POPD & EXIT /B 1)
cmake . || (POPD & EXIT /B 1)
cmake --build . --target ALL_BUILD --config Debug || (POPD & EXIT /B 1)
cmake --build . --target ALL_BUILD --config Release || (POPD & EXIT /B 1)
POPD
EXIT /B 0

:BuildQuickfix
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
cmake . || EXIT /B 1
cmake --build . --target quickfix --config Debug
cmake --build . --target quickfix --config Release
MD "include\quickfix"
xcopy /s src\C++\* include\quickfix >NUL
EXIT /B 0

:CheckCache
SET "CACHE_NAME=%~1"
SET "SETUP_HASH="
FOR /F "skip=1" %%H IN ('certutil -hashfile "%~dp0setup.bat" SHA256') DO (
  IF NOT DEFINED SETUP_HASH SET "SETUP_HASH=%%H"
)
IF EXIST "cache_files\!CACHE_NAME!.txt" (
  SET /P CACHED_HASH=<"cache_files\!CACHE_NAME!.txt"
  IF "!SETUP_HASH!"=="!CACHED_HASH!" EXIT /B 1
)
EXIT /B 0

:Commit
IF NOT EXIST cache_files (
  MD cache_files || EXIT /B 1
)
>"cache_files\!CACHE_NAME!.txt" ECHO !SETUP_HASH!
EXIT /B 0

:SetupVSEnvironment
SET "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
FOR /F "usebackq delims=" %%i IN (` ^
    "!VSWHERE!" -prerelease -latest -property installationPath`) DO (
  IF EXIST "%%i\Common7\Tools\vsdevcmd.bat" (
    CALL "%%i\Common7\Tools\vsdevcmd.bat" -arch=amd64
  )
)
EXIT /B 0

:AddDependency
IF NOT DEFINED NEXT_DEPENDENCY_INDEX SET "NEXT_DEPENDENCY_INDEX=0"
SET "DEPENDENCIES[%NEXT_DEPENDENCY_INDEX%].NAME=%~1"
SET "DEPENDENCIES[%NEXT_DEPENDENCY_INDEX%].URL=%~2"
SET "DEPENDENCIES[%NEXT_DEPENDENCY_INDEX%].HASH=%~3"
SET "DEPENDENCIES[%NEXT_DEPENDENCY_INDEX%].BUILD=%~4"
SET /A NEXT_DEPENDENCY_INDEX+=1
EXIT /B 0

:AddRepo
IF NOT DEFINED NEXT_REPO_INDEX SET "NEXT_REPO_INDEX=0"
SET "REPOS[%NEXT_REPO_INDEX%].NAME=%~1"
SET "REPOS[%NEXT_REPO_INDEX%].URL=%~2"
SET "REPOS[%NEXT_REPO_INDEX%].COMMIT=%~3"
SET "REPOS[%NEXT_REPO_INDEX%].BUILD=%~4"
SET /A NEXT_REPO_INDEX+=1
EXIT /B 0

:InstallDependencies
SET "I=0"
:InstallDependenciesLoop
IF NOT DEFINED DEPENDENCIES[%I%].NAME EXIT /B 0
CALL :DownloadAndExtract "!DEPENDENCIES[%I%].NAME!" "!DEPENDENCIES[%I%].URL!" ^
  "!DEPENDENCIES[%I%].HASH!" "!DEPENDENCIES[%I%].BUILD!" || EXIT /B 1
SET /A I+=1
GOTO InstallDependenciesLoop

:InstallRepos
SET "I=0"
:InstallReposLoop
IF NOT DEFINED REPOS[%I%].NAME EXIT /B 0
CALL :CloneOrUpdateRepo "!REPOS[%I%].NAME!" "!REPOS[%I%].URL!" ^
  "!REPOS[%I%].COMMIT!" "!REPOS[%I%].BUILD!" || EXIT /B 1
SET /A I+=1
GOTO InstallReposLoop

:DownloadAndExtract
SET "FOLDER=%~1"
SET "URL=%~2"
SET "EXPECTED_HASH=%~3"
SET "BUILD_LABEL=%~4"
SET "ACTUAL_HASH="
FOR /F "tokens=* delims=/" %%A IN ("!URL!") DO (
  SET "ARCHIVE=%%~nxA"
)
IF EXIST "!FOLDER!" (
  EXIT /B 0
)
IF NOT EXIST "!ARCHIVE!" (
  curl -fsL -o "!ARCHIVE!" "!URL!" || EXIT /B 1
)
FOR /F "skip=1 tokens=*" %%H IN ('certutil -hashfile "!ARCHIVE!" SHA256') DO (
  IF NOT DEFINED ACTUAL_HASH SET "ACTUAL_HASH=%%H"
)
SET "ACTUAL_HASH=!ACTUAL_HASH: =!"
IF /I NOT "!ACTUAL_HASH!"=="!EXPECTED_HASH!" (
  ECHO Error: SHA256 mismatch for !ARCHIVE!.
  ECHO   Expected: !EXPECTED_HASH!
  ECHO   Actual:   !ACTUAL_HASH!
  DEL /F /Q "!ARCHIVE!"
  SET "ACTUAL_HASH="
  EXIT /B 1
)
SET "ACTUAL_HASH="
MD "!FOLDER!" || EXIT /B 1
powershell -NoProfile -Command ^
  "$ProgressPreference = 'SilentlyContinue'; " ^
  "if ('!ARCHIVE!' -match '\.zip$') { " ^
  "  Expand-Archive -Path '!ARCHIVE!' -DestinationPath '!FOLDER!' -Force " ^
  "} else { " ^
  "  tar -xf '!ARCHIVE!' -C '!FOLDER!' " ^
  "}"
IF ERRORLEVEL 1 (
  RD /S /Q "!FOLDER!" >NUL 2>NUL
  EXIT /B 1
)
SET "DIR_COUNT=0"
SET "FILE_COUNT=0"
SET "SINGLE_DIR="
FOR /D %%D IN ("!FOLDER!\*") DO (
  SET /A DIR_COUNT+=1
  SET "SINGLE_DIR=%%~nxD"
)
FOR %%F IN ("!FOLDER!\*") DO (
  SET /A FILE_COUNT+=1
)
IF "!DIR_COUNT!"=="1" IF "!FILE_COUNT!"=="0" (
  FOR /F "delims=" %%D IN ('DIR /AD /B "!FOLDER!\!SINGLE_DIR!" 2^>NUL') DO (
    MOVE "!FOLDER!\!SINGLE_DIR!\%%D" "!FOLDER!" >NUL
  )
  FOR /F "delims=" %%F IN ('DIR /A-D /B "!FOLDER!\!SINGLE_DIR!" 2^>NUL') DO (
    MOVE "!FOLDER!\!SINGLE_DIR!\%%F" "!FOLDER!" >NUL
  )
  RD /S /Q "!FOLDER!\!SINGLE_DIR!" 2>NUL
)
IF DEFINED BUILD_LABEL (
  PUSHD "!FOLDER!"
  CALL !BUILD_LABEL!
  SET "BUILD_RESULT=!ERRORLEVEL!"
  POPD
  IF NOT "!BUILD_RESULT!"=="0" EXIT /B !BUILD_RESULT!
)
DEL /F /Q "!ARCHIVE!"
EXIT /B 0

:CloneOrUpdateRepo
SET "REPO_NAME=%~1"
SET "REPO_URL=%~2"
SET "REPO_COMMIT=%~3"
SET "BUILD_LABEL=%~4"
SET "NEEDS_BUILD=0"
IF NOT EXIST "!REPO_NAME!" (
  git clone "!REPO_URL!" "!REPO_NAME!"
  IF ERRORLEVEL 1 (
    RD /S /Q "!REPO_NAME!" >NUL 2>NUL
    EXIT /B 1
  )
  PUSHD "!REPO_NAME!"
  git checkout "!REPO_COMMIT!"
  POPD
  SET "NEEDS_BUILD=1"
) ELSE (
  PUSHD "!REPO_NAME!"
  git merge-base --is-ancestor "!REPO_COMMIT!" HEAD
  IF ERRORLEVEL 1 (
    git checkout master
    git pull
    git checkout "!REPO_COMMIT!"
    SET "NEEDS_BUILD=1"
  )
  POPD
)
IF "!NEEDS_BUILD!"=="1" (
  IF DEFINED BUILD_LABEL (
    PUSHD "!REPO_NAME!"
    CALL !BUILD_LABEL!
    SET "BUILD_RESULT=!ERRORLEVEL!"
    POPD
    IF NOT "!BUILD_RESULT!"=="0" EXIT /B !BUILD_RESULT!
  )
)
EXIT /B 0

@ECHO OFF
SETLOCAL EnableDelayedExpansion
FOR /F "delims==" %%V IN ('SET DEPENDENCIES[ 2^>NUL') DO (
  SET "%%V="
)
SET "NEXT_DEPENDENCY_INDEX=0"
FOR /F "delims==" %%V IN ('SET REPOS[ 2^>NUL') DO (
  SET "%%V="
)
SET "NEXT_REPO_INDEX=0"
SET "ROOT=%cd%"
SET "CACHE_DIRECTORY=!ROOT!\cache_files\nexus"
IF NOT EXIST "!CACHE_DIRECTORY!" (
  MD "!CACHE_DIRECTORY!" || EXIT /B 1
)
CALL :SetupVSEnvironment || EXIT /B 1
SET "LUA_HASH="
FOR /F "skip=1" %%H IN ('
    certutil -hashfile "%~dp0Config\lua.cmake" SHA256') DO (
  IF NOT DEFINED LUA_HASH SET "LUA_HASH=%%H"
)
IF NOT DEFINED LUA_HASH EXIT /B 1
CALL :AddRepo "Beam" ^
  "https://www.github.com/spiretrading/beam" ^
  "2321cfc45ceb7d2af35ec48c9938f48aa8c703d2" 1 ^
  ":BuildBeam"
CALL :InstallRepos || EXIT /B 1
SET "PATH=!ROOT!\Strawberry\perl\bin;!PATH!"
SET "PATH=!ROOT!\Strawberry\perl\site\bin;!PATH!"
SET "PATH=!PATH!;!ROOT!\Strawberry\c\bin"
CALL :InstallQt 1 || EXIT /B 1
SET "QUICKFIX_URL=https://github.com/quickfix/quickfix/archive"
SET "QUICKFIX_COMMIT=2ce8a60667d95a55cdc57a210f165e19cb757126"
CALL :AddDependency "lua-5.5.0" ^
  "https://www.lua.org/ftp/lua-5.5.0.tar.gz" ^
  "57ccc32bbbd005cab75bcc52444052535af691789dba2b9016d5c50640d68b3d" ^
  "1 !LUA_HASH!" ":BuildLua"
CALL :AddDependency "quickfix-v.1.16.0" ^
  "!QUICKFIX_URL!/!QUICKFIX_COMMIT!.zip" ^
  "b6fcea5402b443e71c751132938b8ef83efcd0167e005f5bbab103b1875614d1" 1 ^
  ":BuildQuickfix"
CALL :AddDependency "hat-trie-0.7.0" ^
  "https://github.com/Tessil/hat-trie/archive/refs/tags/v0.7.0.zip" ^
  "8ea5441c06fd5d9de1ec8725bf762025a63f931949b9f49d211ab76a75ced68f" 1
CALL :InstallDependencies || EXIT /B 1
CALL :InstallGitPython || EXIT /B 1
EXIT /B 0
ENDLOCAL

:BuildBeam
CALL build.bat Debug -DD="!ROOT!" || EXIT /B 1
CALL build.bat Release -DD="!ROOT!" || EXIT /B 1
EXIT /B 0

:InstallQt
SET "BUILD_HASH=qt-5.15.13 windows-%~1"
SET "BUILD_MARKER=!CACHE_DIRECTORY!\qt-5.15.13.build_complete"
SET "CACHED_HASH="
IF EXIST "!BUILD_MARKER!" (
  SET /P CACHED_HASH=<"!BUILD_MARKER!"
  IF EXIST "qt-5.15.13\" (
    IF "!CACHED_HASH!"=="!BUILD_HASH!" EXIT /B 0
  )
  DEL /F /Q "!BUILD_MARKER!"
  IF EXIST "!BUILD_MARKER!" EXIT /B 1
)
IF NOT EXIST "qt-5.15.13" (
  git clone --branch v5.15.13-lts-lgpl --depth 1 ^
    https://code.qt.io/qt/qt5.git qt-5.15.13 || EXIT /B 1
)
PUSHD qt-5.15.13 || EXIT /B 1
SET "QT_INITIALIZED=1"
FOR %%M IN (qtbase qtsvg qttools qttranslations) DO (
  IF NOT EXIST "%%M\.git" SET "QT_INITIALIZED=0"
  IF NOT EXIST "%%M\%%M.pro" SET "QT_INITIALIZED=0"
)
git diff --quiet --ignore-submodules=dirty -- ^
  qtbase qtsvg qttools qttranslations
IF ERRORLEVEL 1 SET "QT_INITIALIZED=0"
IF "!QT_INITIALIZED!"=="0" (
  perl init-repository --force ^
    --module-subset=qtbase,qtsvg,qttools,qttranslations || (
    POPD
    EXIT /B 1
  )
)
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$ErrorActionPreference = 'Stop';" ^
  "$f = 'qtbase/src/corelib/global/qcompilerdetection.h';" ^
  "$source = [IO.File]::ReadAllText($f);" ^
  "$patched = $source -replace" ^
  "  '(?m)^.*stdext::make_(unchecked|checked)_array_iterator[^\r\n]*', '';" ^
  "if ($source -cne $patched) { [IO.File]::WriteAllText($f, $patched) }" || (
  POPD
  EXIT /B 1
)
CALL configure.bat -prefix "!cd!\qtbase" -opensource -static -mp -make libs ^
  -make tools -nomake examples -nomake tests -opengl desktop ^
  -no-feature-vulkan -no-icu -qt-freetype -qt-harfbuzz -qt-libpng -qt-pcre ^
  -qt-zlib -confirm-license || (POPD & EXIT /B 1)
SETLOCAL
SET "CL=/MP"
nmake || (ENDLOCAL & POPD & EXIT /B 1)
ENDLOCAL
(ECHO !BUILD_HASH!) >"!BUILD_MARKER!" || (POPD & EXIT /B 1)
POPD
EXIT /B 0

:BuildLua
SETLOCAL
SET "CMAKE_GENERATOR="
SET "CMAKE_GENERATOR_PLATFORM="
SET "CMAKE_GENERATOR_TOOLSET="
SET "CMAKE_GENERATOR_INSTANCE="
PUSHD src || (ENDLOCAL & EXIT /B 1)
COPY /Y "%~dp0Config\lua.cmake" CMakeLists.txt >NUL || (
  POPD
  ENDLOCAL
  EXIT /B 1
)
cmake --fresh -S . -B . -A x64 || (POPD & ENDLOCAL & EXIT /B 1)
FOR %%C IN (Debug Release) DO (
  cmake --build . --target lua --config %%C --parallel || (
    POPD
    ENDLOCAL
    EXIT /B 1
  )
)
POPD
ENDLOCAL
EXIT /B 0

:BuildQuickfix
SETLOCAL
SET "CMAKE_GENERATOR="
SET "CMAKE_GENERATOR_PLATFORM="
SET "CMAKE_GENERATOR_TOOLSET="
SET "CMAKE_GENERATOR_INSTANCE="
cmake --fresh -S . -B build -A x64 -DCMAKE_INSTALL_PREFIX="!cd!" ^
  -DQUICKFIX_SHARED_LIBS=OFF -DQUICKFIX_EXAMPLES=OFF -DQUICKFIX_TESTS=OFF ^
  -DSTATIC_RUNTIME=OFF || (ENDLOCAL & EXIT /B 1)
FOR %%C IN (Debug Release) DO (
  cmake --build build --target quickfix --config %%C --parallel || (
    ENDLOCAL
    EXIT /B 1
  )
)
cmake --install build --config Release || (ENDLOCAL & EXIT /B 1)
ENDLOCAL
EXIT /B 0

:InstallGitPython
python -c "import git" >NUL 2>&1
IF NOT ERRORLEVEL 1 EXIT /B 0
python -m pip install --user --quiet GitPython
EXIT /B !ERRORLEVEL!

:SetupVSEnvironment
SET "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
SET "VS_FOUND="
FOR /F "usebackq delims=" %%i IN (` ^
    "!VSWHERE!" -prerelease -latest -products * ^
      -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 ^
      -property installationPath`) DO (
  IF EXIST "%%i\Common7\Tools\vsdevcmd.bat" (
    CALL "%%i\Common7\Tools\vsdevcmd.bat" -no_logo -arch=x64 -host_arch=x64 || (
      EXIT /B 1
    )
    SET "VS_FOUND=1"
  )
)
IF NOT DEFINED VS_FOUND (
  ECHO Error: Visual Studio C++ build tools were not found.
  EXIT /B 1
)
EXIT /B 0

:AddDependency
SET "DEPENDENCIES[%NEXT_DEPENDENCY_INDEX%].NAME=%~1"
SET "DEPENDENCIES[%NEXT_DEPENDENCY_INDEX%].URL=%~2"
SET "DEPENDENCIES[%NEXT_DEPENDENCY_INDEX%].HASH=%~3"
SET "DEPENDENCIES[%NEXT_DEPENDENCY_INDEX%].REVISION=%~4"
SET "DEPENDENCIES[%NEXT_DEPENDENCY_INDEX%].BUILD=%~5"
SET "DEPENDENCIES[%NEXT_DEPENDENCY_INDEX%].STRIP=%~6"
SET /A NEXT_DEPENDENCY_INDEX+=1
EXIT /B 0

:AddRepo
SET "REPOS[%NEXT_REPO_INDEX%].NAME=%~1"
SET "REPOS[%NEXT_REPO_INDEX%].URL=%~2"
SET "REPOS[%NEXT_REPO_INDEX%].COMMIT=%~3"
SET "REPOS[%NEXT_REPO_INDEX%].REVISION=%~4"
SET "REPOS[%NEXT_REPO_INDEX%].BUILD=%~5"
SET /A NEXT_REPO_INDEX+=1
EXIT /B 0

:InstallDependencies
SET "I=0"
:InstallDependenciesLoop
IF NOT DEFINED DEPENDENCIES[%I%].NAME EXIT /B 0
CALL :DownloadAndExtract "!DEPENDENCIES[%I%].NAME!" "!DEPENDENCIES[%I%].URL!" ^
  "!DEPENDENCIES[%I%].HASH!" "!DEPENDENCIES[%I%].REVISION!" ^
  "!DEPENDENCIES[%I%].BUILD!" "!DEPENDENCIES[%I%].STRIP!" || EXIT /B 1
SET /A I+=1
GOTO InstallDependenciesLoop

:InstallRepos
SET "I=0"
:InstallReposLoop
IF NOT DEFINED REPOS[%I%].NAME EXIT /B 0
CALL :CloneOrUpdateRepo "!REPOS[%I%].NAME!" "!REPOS[%I%].URL!" ^
  "!REPOS[%I%].COMMIT!" "!REPOS[%I%].REVISION!" ^
  "!REPOS[%I%].BUILD!" || EXIT /B 1
SET /A I+=1
GOTO InstallReposLoop

:DownloadAndExtract
SET "FOLDER=%~1"
SET "BUILD_MARKER=!CACHE_DIRECTORY!\!FOLDER!.build_complete"
SET "URL=%~2"
SET "EXPECTED_HASH=%~3"
SET "BUILD_HASH=!EXPECTED_HASH! windows-%~4"
SET "BUILD_LABEL=%~5"
SET "STRIP=%~6"
IF NOT DEFINED STRIP SET "STRIP=1"
SET "ACTUAL_HASH="
FOR /F "tokens=* delims=/" %%A IN ("!URL!") DO (
  SET "ARCHIVE=%%~nxA"
)
SET "CACHED_HASH="
IF EXIST "!BUILD_MARKER!" (
  SET /P CACHED_HASH=<"!BUILD_MARKER!"
  IF EXIST "!FOLDER!\" (
    IF "!CACHED_HASH!"=="!BUILD_HASH!" EXIT /B 0
  )
  DEL /F /Q "!BUILD_MARKER!"
  IF EXIST "!BUILD_MARKER!" EXIT /B 1
)
IF EXIST "!FOLDER!\.nexus_extract_complete" (
  SET /P CACHED_HASH=<"!FOLDER!\.nexus_extract_complete"
  IF "!CACHED_HASH!"=="!EXPECTED_HASH!" GOTO BuildDependency
  DEL /F /Q "!FOLDER!\.nexus_extract_complete"
  IF EXIST "!FOLDER!\.nexus_extract_complete" EXIT /B 1
)
IF NOT EXIST "!ARCHIVE!" (
  curl -fsL -o "!ARCHIVE!" "!URL!" || (
    IF EXIST "!ARCHIVE!" DEL /F /Q "!ARCHIVE!"
    EXIT /B 1
  )
)
FOR /F "skip=1 tokens=*" %%H IN ('certutil -hashfile "!ARCHIVE!" SHA256') DO (
  IF NOT DEFINED ACTUAL_HASH SET "ACTUAL_HASH=%%H"
)
SET "ACTUAL_HASH=!ACTUAL_HASH: =!"
IF /I NOT "!ACTUAL_HASH!"=="!EXPECTED_HASH!" (
  ECHO Error: SHA256 mismatch for !ARCHIVE!.
  DEL /F /Q "!ARCHIVE!"
  EXIT /B 1
)
IF NOT EXIST "!FOLDER!" (
  MD "!FOLDER!" || EXIT /B 1
)
cmake -DARCHIVE:FILEPATH="!ARCHIVE!" -DDESTINATION:PATH="!FOLDER!" ^
  -DSTRIP_COMPONENTS=!STRIP! -P "%~dp0Config\extract.cmake" || EXIT /B 1
(ECHO !EXPECTED_HASH!) >"!FOLDER!\.nexus_extract_complete" || EXIT /B 1
:BuildDependency
IF DEFINED BUILD_LABEL (
  PUSHD "!FOLDER!" || EXIT /B 1
  CALL !BUILD_LABEL!
  SET "BUILD_RESULT=!ERRORLEVEL!"
  POPD
  IF NOT "!BUILD_RESULT!"=="0" EXIT /B !BUILD_RESULT!
)
(ECHO !BUILD_HASH!) >"!BUILD_MARKER!" || EXIT /B 1
IF EXIST "!ARCHIVE!" DEL /F /Q "!ARCHIVE!"
EXIT /B 0

:CloneOrUpdateRepo
SET "REPO_NAME=%~1"
SET "BUILD_MARKER=!CACHE_DIRECTORY!\!REPO_NAME!.build_complete"
SET "REPO_URL=%~2"
SET "REPO_COMMIT=%~3"
SET "BUILD_REVISION=%~4"
SET "BUILD_LABEL=%~5"
SET "IS_NEW_REPO="
IF NOT EXIST "!REPO_NAME!" (
  IF EXIST "!BUILD_MARKER!" (
    DEL /F /Q "!BUILD_MARKER!"
    IF EXIST "!BUILD_MARKER!" EXIT /B 1
  )
  git clone "!REPO_URL!" "!REPO_NAME!" || EXIT /B 1
  SET "IS_NEW_REPO=1"
)
PUSHD "!REPO_NAME!" || EXIT /B 1
IF DEFINED IS_NEW_REPO (
  git checkout "!REPO_COMMIT!" || (POPD & EXIT /B 1)
)
git merge-base --is-ancestor "!REPO_COMMIT!" HEAD >NUL 2>NUL
IF ERRORLEVEL 1 (
  git fetch origin || (POPD & EXIT /B 1)
  IF EXIST "!BUILD_MARKER!" (
    DEL /F /Q "!BUILD_MARKER!"
    IF EXIST "!BUILD_MARKER!" (POPD & EXIT /B 1)
  )
  git checkout "!REPO_COMMIT!" || (POPD & EXIT /B 1)
)
SET "REPO_HEAD="
FOR /F %%H IN ('git rev-parse HEAD') DO (
  SET "REPO_HEAD=%%H"
)
IF NOT DEFINED REPO_HEAD (POPD & EXIT /B 1)
SET "BUILD_HASH=!REPO_HEAD! windows-!BUILD_REVISION!"
SET "CACHED_HASH="
IF EXIST "!BUILD_MARKER!" (
  SET /P CACHED_HASH=<"!BUILD_MARKER!"
)
IF NOT "!CACHED_HASH!"=="!BUILD_HASH!" (
  IF EXIST "!BUILD_MARKER!" (
    DEL /F /Q "!BUILD_MARKER!"
    IF EXIST "!BUILD_MARKER!" (POPD & EXIT /B 1)
  )
  IF DEFINED BUILD_LABEL (
    CALL !BUILD_LABEL!
    SET "BUILD_RESULT=!ERRORLEVEL!"
    IF NOT "!BUILD_RESULT!"=="0" (POPD & EXIT /B !BUILD_RESULT!)
  )
  (ECHO !BUILD_HASH!) >"!BUILD_MARKER!" || (POPD & EXIT /B 1)
) ELSE (
  PUSHD "!ROOT!" || (POPD & EXIT /B 1)
  CALL "!ROOT!\!REPO_NAME!\Beam\setup.bat"
  SET "BUILD_RESULT=!ERRORLEVEL!"
  POPD
  IF NOT "!BUILD_RESULT!"=="0" (POPD & EXIT /B !BUILD_RESULT!)
)
POPD
EXIT /B 0

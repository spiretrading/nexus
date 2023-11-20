@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET EXIT_STATUS=0
SET ROOT=%cd%
IF EXIST cache_files\nexus.txt (
  FOR /F %%i IN (
      'ls -l --time-style=full-iso "%~dp0\setup.bat" ^| awk "{print $6 $7}"') DO (
    FOR /F %%j IN (
        'ls -l --time-style=full-iso cache_files\nexus.txt ^| awk "{print $6 $7}"') DO (
      IF "%%i" LSS "%%j" (
        EXIT /B 0
      )
    )
  )
)
SET VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
FOR /f "usebackq delims=" %%i IN (`!VSWHERE! -prerelease -latest -property installationPath`) DO (
  IF EXIST "%%i\Common7\Tools\vsdevcmd.bat" (
    CALL "%%i\Common7\Tools\vsdevcmd.bat"
  )
)
SET BUILD_BEAM=
SET BEAM_COMMIT="00903c5a8bc91596e1c517708724a36af41e0555"
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
IF NOT EXIST qt-5.15.2 (
  git clone git://code.qt.io/qt/qt5.git qt-5.15.2
  IF !ERRORLEVEL! EQU 0 (
    PUSHD qt-5.15.2
    git checkout 5.15.2
    perl init-repository --module-subset=qtbase,qtsvg,qttools,qttranslations
    CALL configure -prefix %cd% -opensource -static -mp -make libs -make tools ^
      -nomake examples -nomake tests -opengl desktop -no-icu -qt-freetype ^
      -qt-harfbuzz -qt-libpng -qt-pcre -qt-zlib -confirm-license
    SET CL=/MP
    nmake
    DEL qtbase\lib\cmake\Qt5Core\Qt5CoreConfigExtrasMkspecDir.cmake
    COPY NUL qtbase\lib\cmake\Qt5Core\Qt5CoreConfigExtrasMkspecDir.cmake
    POPD
  ) ELSE (
    RD /S /Q qt-5.15.2
    SET EXIT_STATUS=1
  )
)
IF NOT EXIST lua-5.4.4 (
  wget http://www.lua.org/ftp/lua-5.4.4.tar.gz --no-check-certificate
  IF !ERRORLEVEL! LEQ 0 (
    gzip -d -c lua-5.4.4.tar.gz | tar -xf -
    PUSHD lua-5.4.4\src
    COPY %~dp0\Config\lua.cmake CMakeLists.txt
    cmake -A Win32 .
    cmake --build . --target ALL_BUILD --config Debug
    cmake --build . --target ALL_BUILD --config Release
    POPD
  ) ELSE (
    SET EXIT_STATUS=1
  )
  DEL /F /Q lua-5.4.4.tar.gz
)
IF NOT EXIST quickfix-v.1.15.1 (
  wget https://github.com/quickfix/quickfix/archive/49b3508e48f0bbafbab13b68be72250bdd971ac2.zip -O quickfix-v.1.15.1.zip --no-check-certificate
  IF !ERRORLEVEL! LEQ 0 (
    tar -xf quickfix-v.1.15.1.zip
    mv quickfix-49b3508e48f0bbafbab13b68be72250bdd971ac2 quickfix-v.1.15.1
    PUSHD quickfix-v.1.15.1
    PUSHD src\C++
    sed -i "105s/.*/template<typename T> using SmartPtr = std::shared_ptr<T>;/" Utility.h
    sed -i "108s/.*/template<typename T> using SmartPtr = std::shared_ptr<T>;/" Utility.h
    sed -i "s/std::auto_ptr/std::shared_ptr/" DOMDocument.h
    sed -i "s/std::auto_ptr/std::shared_ptr/" Message.h
    sed -i "s/std::auto_ptr/std::shared_ptr/" Message.cpp
    sed -i "s/pGroup\.release()/new Group(\*pGroup)/" Message.cpp
    POPD
    devenv /Upgrade quickfix_vs12.sln
    msbuild quickfix_vs12.sln /p:PlatformToolset=v143 /p:configuration=Debug ^
      /p:UseEnv=true
    msbuild quickfix_vs12.sln /p:PlatformToolset=v143 /p:configuration=Release ^
      /p:UseEnv=true
    POPD
  ) ELSE (
    SET EXIT_STATUS=1
  )
  DEL /F /Q quickfix-v.1.15.1.zip
)
ECHO timestamp > cache_files\nexus.txt
ENDLOCAL
EXIT /B !EXIT_STATUS!

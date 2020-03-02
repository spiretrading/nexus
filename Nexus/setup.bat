@ECHO OFF
SETLOCAL
SET ROOT=%cd%
SET VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
FOR /f "usebackq delims=" %%i IN (`%VSWHERE% -prerelease -latest -property installationPath`) DO (
  IF EXIST "%%i\Common7\Tools\vsdevcmd.bat" (
    CALL "%%i\Common7\Tools\vsdevcmd.bat"
  )
)
SET BUILD_BEAM=
IF NOT EXIST Beam (
  git clone https://www.github.com/eidolonsystems/beam Beam
  SET BUILD_BEAM=1
)
SET beam_commit="469c1a9e6e456440055e54c7d06642fa763f29ed"
PUSHD Beam
git merge-base --is-ancestor "%beam_commit%" HEAD
IF NOT "%ERRORLEVEL%" == "0" (
  git checkout master
  git pull
  git checkout "%beam_commit%"
  SET BUILD_BEAM=1
)
IF "%BUILD_BEAM%" == "1" (
  CALL configure.bat -DD="%ROOT%"
  CALL build.bat Debug
  CALL build.bat Release
) ELSE (
  PUSHD %ROOT%
  CALL Beam\Beam\setup.bat
  POPD
)
POPD
SET commit=
SET PATH=%PATH%;%ROOT%\Strawberry\perl\site\bin;%ROOT%\Strawberry\perl\bin;%ROOT%\Strawberry\c\bin
IF NOT EXIST qt-5.14.0 (
  git clone git://code.qt.io/qt/qt5.git qt-5.14.0
  IF EXIST qt-5.14.0 (
    PUSHD qt-5.14.0
    git checkout 5.14.0
    perl init-repository --module-subset=default
    CALL configure -prefix %cd% -opensource -static -mp -make libs -make tools ^
      -nomake examples -nomake tests -opengl desktop -no-icu -qt-freetype ^
      -qt-harfbuzz -qt-libpng -qt-pcre -qt-zlib -confirm-license
    SET CL=/MP
    nmake
    DEL qtbase\lib\cmake\Qt5Core\Qt5CoreConfigExtrasMkspecDir.cmake
    COPY NUL qtbase\lib\cmake\Qt5Core\Qt5CoreConfigExtrasMkspecDir.cmake
    POPD
  )
)
IF NOT EXIST lua-5.3.5 (
  wget http://www.lua.org/ftp/lua-5.3.5.tar.gz --no-check-certificate
  IF EXIST lua-5.3.5.tar.gz (
    gzip -d -c lua-5.3.5.tar.gz | tar -xf -
    PUSHD lua-5.3.5\src
    COPY %~dp0\Config\lua.cmake CMakeLists.txt
    cmake -A Win32 .
    cmake --build . --target ALL_BUILD --config Debug
    cmake --build . --target ALL_BUILD --config Release
    POPD
    DEL lua-5.3.5.tar.gz
  )
)
IF NOT EXIST quickfix-v.1.15.1 (
  wget https://github.com/quickfix/quickfix/archive/49b3508e48f0bbafbab13b68be72250bdd971ac2.zip -O quickfix-v.1.15.1.zip --no-check-certificate
  IF EXIST quickfix-v.1.15.1.zip (
    unzip quickfix-v.1.15.1.zip
    mv quickfix-49b3508e48f0bbafbab13b68be72250bdd971ac2 quickfix-v.1.15.1
    PUSHD quickfix-v.1.15.1
    PUSHD src\C++
    sed -i "105s/.*/template<typename T> using SmartPtr = std::shared_ptr<T>;/" Utility.h
    sed -i "108s/.*/template<typename T> using SmartPtr = std::shared_ptr<T>;/" Utility.h
    POPD
    devenv /Upgrade quickfix_vs12.sln
    msbuild quickfix_vs12.sln /p:PlatformToolset=v142 /p:configuration=Debug ^
      /p:UseEnv=true
    msbuild quickfix_vs12.sln /p:PlatformToolset=v142 /p:configuration=Release ^
      /p:UseEnv=true
    POPD
    DEL quickfix-v.1.15.1.zip
  )
)
ENDLOCAL

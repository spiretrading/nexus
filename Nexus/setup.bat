@ECHO OFF
SETLOCAL
SET ROOT="%cd%"
SET BUILD_BEAM=
IF NOT EXIST Beam (
  git clone https://www.github.com/eidolonsystems/beam Beam
  SET BUILD_BEAM=1
)
SET beam_commit="937ac6ef0fbc0fb561f71a31dfe0bbc039bcf093"
PUSHD Beam
git merge-base --is-ancestor "%beam_commit%" HEAD
IF NOT "%ERRORLEVEL%" == "0" (
  git checkout master
  git pull
  git checkout "%beam_commit%"
  SET BUILD_BEAM=1
)
IF "%BUILD_BEAM%" == "1" (
  CALL configure.bat "-DD=%ROOT%"
  CALL build.bat Debug
  CALL build.bat Release
)
POPD
SET commit=
IF NOT EXIST qt-5.11.2 (
  git clone git://code.qt.io/qt/qt5.git qt-5.11.2
  IF EXIST qt-5.11.2 (
    PUSHD qt-5.11.2
    git checkout 5.11.2
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
ENDLOCAL

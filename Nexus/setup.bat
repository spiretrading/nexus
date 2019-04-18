@ECHO OFF
SETLOCAL
SET ROOT="%cd%"
IF NOT EXIST Beam (
  git clone https://www.github.com/eidolonsystems/beam Beam
)
SET beam_commit="e82694fca1ec7336012998c5f0dbfe0468199680"
PUSHD Beam
FOR /f "usebackq tokens=*" %%a IN (`git log -1 ^| head -1 ^| awk "{ print $2 }"`) DO SET commit=%%a
IF NOT "%commit%" == "%beam_commit%" (
  git checkout master
  git pull
  git checkout %beam_commit%
)
CALL Beam\run_cmake.bat "-DD=%ROOT%"
PUSHD Beam
CALL build.bat Debug
CALL build.bat Release
POPD
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
    set CL=/MP
    nmake
    POPD
  )
)
ENDLOCAL

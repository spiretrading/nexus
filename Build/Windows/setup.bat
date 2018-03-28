SETLOCAL

SET expected_commit="39cde4710261bcffcc6cc04c925ca251dddb047d"
if exist Beam goto end_beam_setup
  git clone https://www.github.com/eidolonsystems/beam.git Beam
  pushd Beam
  git checkout %expected_commit%
  popd
  call Beam\Build\Windows\setup.bat
  pushd Beam\Build\Windows
  call run_cmake.bat
  call build.bat
  popd
:end_beam_setup

if not exist Beam goto end_beam_pull
  pushd Beam
  for /f "usebackq tokens=*" %%a in (`git log -1 ^| head -1 ^| awk "{ print $2 }"`) do SET commit=%%a
  if not "%commit%" == %expected_commit% (
    git checkout master
    git pull
    git checkout %expected_commit%
    popd
    call Beam\Build\Make\setup.bat
    pushd Beam\Build\Windows
    call run_cmake.bat
    call build.bat
  )
  popd
:end_beam_pull

if exist quickfix goto end_quick_fix_setup
  wget --no-check-certificate http://prdownloads.sourceforge.net/quickfix/quickfix-1.14.3.zip
  if not exist quickfix-1.14.3.zip goto end_quick_fix_setup
    unzip quickfix-1.14.3.zip
    pushd quickfix
    devenv /Upgrade quickfix_vs12.sln
    popd
    pushd quickfix\src\C++
    cat Utility.cpp | sed "s/  result = _beginthreadex( NULL, 0, &func, var, 0, &id );/  result = _beginthreadex( NULL, 0, reinterpret_cast<_beginthreadex_proc_type>(\&func), var, 0, \&id );/" > Utility.cpp.new
    mv Utility.cpp.new Utility.cpp
    popd
    pushd quickfix
    msbuild quickfix_vs12.sln /p:PlatformToolset=v141 /p:configuration=Debug ^
      /p:UseEnv=true
    msbuild quickfix_vs12.sln /p:PlatformToolset=v141 /p:configuration=Release ^
      /p:UseEnv=true
    popd
    rm quickfix-1.14.3.zip
:end_quick_fix_setup

if exist qt-5.10.0 goto end_qt_setup
  git clone git://code.qt.io/qt/qt5.git qt-5.10.0
  if not exist qt-5.10.0 goto end_qt_setup
    pushd qt-5.10.0
    git checkout v5.10.0
    perl init-repository ^
      --module-subset=default,-qtgamepad,-qtmultimedia,-qtspeech,-qtvirualkeyboard,-qtwebchannel,-qtwebengine,-qtwebglplugin,-qtwebkit,-qtwebkit-examples,-qtwebsockets,-qtwebview
    call configure -opensource -static -mp -make libs -make tools ^
      -nomake examples -nomake tests -gif -opengl desktop -no-icu -qt-freetype ^
      -qt-harfbuzz -qt-libpng -qt-pcre -qt-zlib -confirm-license
    set CL=/MP
    nmake
    popd
:end_qt_setup

CALL %~dp0../../Applications/ClientWebPortal/Build/Windows/setup.bat

ENDLOCAL

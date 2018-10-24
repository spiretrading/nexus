SETLOCAL

SET beam_commit="a5c0455840415a5296b50b5f548f036892d0a698"
if exist Beam goto end_beam_setup
  git clone https://www.github.com/eidolonsystems/beam.git Beam
  pushd Beam
  git checkout %beam_commit%
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
  if not "%commit%" == %beam_commit% (
    git checkout master
    git pull
    git checkout %beam_commit%
    popd
    call Beam\Build\Make\setup.bat
    pushd Beam\Build\Windows
    call run_cmake.bat
    call build.bat
  )
  popd
:end_beam_pull

if exist quickfix-v.1.15.1 goto end_quick_fix_setup
  wget https://github.com/quickfix/quickfix/archive/49b3508e48f0bbafbab13b68be72250bdd971ac2.zip -O quickfix-v.1.15.1.zip --no-check-certificate
  if not exist quickfix-v.1.15.1.zip goto end_quick_fix_setup
    unzip quickfix-v.1.15.1.zip
    mv quickfix-49b3508e48f0bbafbab13b68be72250bdd971ac2 quickfix-v.1.15.1
    pushd quickfix-v.1.15.1
    pushd src\C++
    sed -i "105s/.*/template<typename T> using SmartPtr = std::shared_ptr<T>;/" Utility.h
    sed -i "108s/.*/template<typename T> using SmartPtr = std::shared_ptr<T>;/" Utility.h
    sed -i "s/  result = _beginthreadex( NULL, 0, &func, var, 0, &id );/  result = _beginthreadex( NULL, 0, reinterpret_cast<_beginthreadex_proc_type>(\&func), var, 0, \&id );/"
    popd
    devenv /Upgrade quickfix_vs12.sln
    msbuild quickfix_vs12.sln /p:PlatformToolset=v141 /p:configuration=Debug ^
      /p:UseEnv=true
    msbuild quickfix_vs12.sln /p:PlatformToolset=v141 /p:configuration=Release ^
      /p:UseEnv=true
    popd
    rm quickfix-v.1.15.1.zip
:end_quick_fix_setup

if exist qt-5.11.2 goto end_qt_setup
  git clone git://code.qt.io/qt/qt5.git qt-5.11.2
  if not exist qt-5.11.2 goto end_qt_setup
    pushd qt-5.11.2
    git checkout 5.11.2
    perl init-repository --module-subset=default
    call configure -prefix %cd% -opensource -static -mp -make libs -make tools ^
      -nomake examples -nomake tests -opengl desktop -no-icu -qt-freetype ^
      -qt-harfbuzz -qt-libpng -qt-pcre -qt-zlib -confirm-license
    set CL=/MP
    nmake
    popd
:end_qt_setup

ENDLOCAL

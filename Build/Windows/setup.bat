SETLOCAL

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
    msbuild quickfix_vs12.sln /p:PlatformToolset=v141 /p:configuration=Debug /p:UseEnv=true
    msbuild quickfix_vs12.sln /p:PlatformToolset=v141 /p:configuration=Release /p:UseEnv=true
    popd
    rm quickfix-1.14.3.zip
:end_quick_fix_setup

if exist qt-5.10.0 goto end_qt_setup
  git clone git://code.qt.io/qt/qt5.git qt-5.10.0
  if not exist qt-5.10.0 goto end_qt_setup
    pushd qt-5.10.0
    git checkout v5.10.0
    perl init-repository --module-subset=default,-qtwebkit,-qtwebkit-examples,-qtwebengine
    echo y >> accept
    call configure -opensource -static -make libs -make tools -opengl desktop -no-icu -qt-zlib -mp < accept
    set CL=/MP
    nmake
    rm accept
    popd
:end_qt_setup

CALL %~dp0../../Applications/ClientWebPortal/Build/Windows/setup.bat

ENDLOCAL

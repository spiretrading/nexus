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
    msbuild quickfix_vs12.sln /p:PlatformToolset=v140 /p:configuration=Debug /p:UseEnv=true
    msbuild quickfix_vs12.sln /p:PlatformToolset=v140 /p:configuration=Release /p:UseEnv=true
    popd
    rm quickfix-1.14.3.zip
:end_quick_fix_setup

if exist qt-5.5.0 goto end_qt_setup
  wget --no-check-certificate https://download.qt.io/archive/qt/5.5/5.5.0/single/qt-everywhere-opensource-src-5.5.0.zip
  if not exist qt-everywhere-opensource-src-5.5.0.zip goto end_qt_setup
    unzip qt-everywhere-opensource-src-5.5.0.zip
    mv qt-everywhere-opensource-src-5.5.0 qt-5.5.0
    pushd qt-5.5.0
    echo y >> accept
    start /wait cmd.exe @cmd /k "configure -opensource -static -make libs -make tools -opengl desktop -no-icu -qt-zlib -mp < accept & exit"
    set CL=/MP
    nmake
    rm accept
    popd
    rm qt-everywhere-opensource-src-5.5.0.zip
:end_qt_setup

CALL %~dp0../../Applications/ClientWebPortal/Build/Windows/setup.bat

ENDLOCAL

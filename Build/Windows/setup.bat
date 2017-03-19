SETLOCAL

if exist quickfix goto end_quick_fix_setup
  wget --no-check-certificate http://prdownloads.sourceforge.net/quickfix/quickfix-1.14.3.zip
  if not exist quickfix-1.14.3.zip goto end_quick_fix_setup
    unzip quickfix-1.14.3.zip
    cd quickfix
    devenv /Upgrade quickfix_vs12.sln
    cd src/C++
    cat Utility.cpp | sed "s/  result = _beginthreadex( NULL, 0, &func, var, 0, &id );/  result = _beginthreadex( NULL, 0, reinterpret_cast<_beginthreadex_proc_type>(\&func), var, 0, \&id );/" > Utility.cpp.new
    mv Utility.cpp.new Utility.cpp
    cd ../../
    msbuild quickfix_vs12.sln /p:PlatformToolset=v140 /p:configuration=Debug /p:UseEnv=true
    msbuild quickfix_vs12.sln /p:PlatformToolset=v140 /p:configuration=Release /p:UseEnv=true
    cd ..
    rm quickfix-1.14.3.zip
:end_quick_fix_setup

if exist qt-5.5.0 goto end_qt_setup
  wget --no-check-certificate https://download.qt.io/archive/qt/5.5/5.5.0/single/qt-everywhere-opensource-src-5.5.0.zip
  if not exist qt-everywhere-opensource-src-5.5.0.zip goto end_qt_setup
    unzip qt-everywhere-opensource-src-5.5.0.zip
    mv qt-everywhere-opensource-src-5.5.0 qt-5.5.0
    cd qt-5.5.0
    echo y >> accept
    configure -opensource -static -make libs -make tools -opengl desktop -no-icu -qt-zlib -mp < accept
    (
      echo set CL=/MP
      echo nmake
    ) | cmd.exe
    nmake
    rm accept
    cd ..
    rm qt-everywhere-opensource-src-5.5.0.zip
:end_qt_setup

if exist sqlite goto end_sqlite_setup
  wget http://www.sqlite.org/2015/sqlite-amalgamation-3080803.zip
  if not exist sqlite-amalgamation-3080803.zip goto end_sqlite_setup
    unzip sqlite-amalgamation-3080803.zip
    mv sqlite-amalgamation-3080803 sqlite
    cd sqlite
    cl /c /O2 sqlite3.c
    lib sqlite3.obj
    cd ..
    rm sqlite-amalgamation-3080803.zip
:end_sqlite_setup

CALL %~dp0../../Applications/ClientWebPortal/Build/Windows/setup.bat

ENDLOCAL

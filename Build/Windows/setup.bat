SETLOCAL

if exist quickfix goto end_quick_fix_setup
wget http://prdownloads.sourceforge.net/quickfix/quickfix-1.14.3.zip
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
wget http://download.qt.io/official_releases/qt/5.5/5.5.0/single/qt-everywhere-opensource-src-5.5.0.zip
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
unzip sqlite-amalgamation-3080803.zip
mv sqlite-amalgamation-3080803 sqlite
cd sqlite
cl /c /O2 sqlite3.c
lib sqlite3.obj
cd ..
rm sqlite-amalgamation-3080803.zip
:end_sqlite_setup

if exist react-0.14.7 goto end_react_setup
mkdir react-0.14.7
cd react-0.14.7
wget https://fb.me/react-0.14.7.js --no-check-certificate -O react-0.14.7.js
wget https://fb.me/react-dom-0.14.7.js --no-check-certificate -O react-dom-0.14.7.js
cd ..
:end_react_setup
  
if exist requirejs-2.1.22 goto end_requirejs_setup
mkdir requirejs-2.1.22
cd requirejs-2.1.22
wget http://requirejs.org/docs/release/2.1.22/minified/require.js
cd ..
:end_requirejs_setup

if exist require-css goto end_require_css_setup
wget https://github.com/guybedford/require-css/archive/master.zip
unzip master.zip
mv require-css-master require-css
rm master.zip
:end_require_css_setup

CALL %~dp0../../Applications/ClientWebPortal/Build/Windows/setup.bat

ENDLOCAL

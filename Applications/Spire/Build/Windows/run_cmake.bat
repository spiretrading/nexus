SETLOCAL
CALL %~dp0version.bat
CALL %~dp0set_env.bat
set INSTALL_DIRECTORY=%~dp0../../Application
set LIB_INSTALL_DIRECTORY=%~dp0../../Library
set TEST_INSTALL_DIRECTORY=%~dp0../../Tests
pushd %~dp0
echo set(_qt5_corelib_extra_includes "${_qt5Core_install_prefix}//mkspecs/win32-msvc") > %QTDIR%/qtbase/lib/cmake/install/Qt5Core/Qt5CoreConfigExtrasMkspecDir.cmake
cmake -T host=x64 -DCMAKE_PREFIX_PATH=%QTDIR%/qtbase/lib/cmake %~dp0../Config
popd
ENDLOCAL

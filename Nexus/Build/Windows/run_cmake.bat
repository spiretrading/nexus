SETLOCAL
CALL %~dp0set_env.bat
set LIB_INSTALL_DIRECTORY=%~dp0../../Library
set TEST_INSTALL_DIRECTORY=%~dp0../../Tests
pushd %~dp0
cmake %~dp0../Config
touch Python/Python.vcxproj.new
cp Python/Python.vcxproj Python/Python.vcxproj.base
sed "0,/<\/ItemGroup>/{s/  <\/ItemGroup>/  <\/ItemGroup>\n  <PropertyGroup>\n    <UseNativeEnvironment>true<\/UseNativeEnvironment>\n  <\/PropertyGroup>\n/}" < Python/Python.vcxproj.base > Python/Python.vcxproj.base.new
@echo off
for /f "delims=" %%i in ('diff Python/Python.vcxproj.base.new Python/Python.vcxproj.new') do set output=%%i
@echo on
if not "%output%"=="" (
  cp Python/Python.vcxproj.base.new Python/Python.vcxproj.new
  cp Python/Python.vcxproj.new Python/Python.vcxproj
)
popd
ENDLOCAL

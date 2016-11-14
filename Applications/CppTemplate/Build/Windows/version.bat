cd %~dp0../..
mkdir Include
cd Include
mkdir CppTemplate
cd %~dp0
printf "#define CPP_TEMPLATE_VERSION """> %~dp0../../Include/CppTemplate/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/CppTemplate/Version.hpp
printf """\n" >> %~dp0../../Include/CppTemplate/Version.hpp

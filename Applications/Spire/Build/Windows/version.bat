cd %~dp0../..
mkdir Include
cd Include
mkdir Spire
cd %~dp0
printf "#define SPIRE_VERSION """> %~dp0../../Include/Spire/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/Spire/Version.hpp
printf """\n" >> %~dp0../../Include/Spire/Version.hpp

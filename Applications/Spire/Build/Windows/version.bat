cd %~dp0../..
mkdir include
cd include
mkdir spire
cd %~dp0
printf "#define SPIRE_VERSION """> %~dp0../../include/spire/version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../include/spire/version.hpp
printf """\n" >> %~dp0../../include/spire/version.hpp

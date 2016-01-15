printf "#define SPIRE_VERSION """> %~dp0../../Include/Spire/Version.hpp
hg id -n | tr -d "\n\" >> %~dp0../../Include/Spire/Version.hpp
printf """" >> %~dp0../../Include/Spire/Version.hpp

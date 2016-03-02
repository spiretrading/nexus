printf "#define SPIRE_VERSION """> %~dp0../../Include/Spire/Version.hpp
for /f "delims=" %%i in ('git rev-list --count --first-parent HEAD') do set output=%%i
expr 3000 + %output% | tr -d "\r\n" >> %~dp0../../Include/Spire/Version.hpp
printf """\n" >> %~dp0../../Include/Spire/Version.hpp

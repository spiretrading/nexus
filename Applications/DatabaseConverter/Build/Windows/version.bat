pushd %~dp0..\..
mkdir Include
pushd Include
mkdir DatabaseConverter
popd
popd
printf "#define DATABASE_CONVERTER_VERSION """> %~dp0../../Include/DatabaseConverter/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/DatabaseConverter/Version.hpp
printf """\n" >> %~dp0../../Include/DatabaseConverter/Version.hpp

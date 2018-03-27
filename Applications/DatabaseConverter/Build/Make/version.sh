#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/DatabaseConverter
printf "#define DATABASE_CONVERTER_VERSION \""> ./../../Include/DatabaseConverter/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/DatabaseConverter/Version.hpp
printf \""\n" >> ./../../Include/DatabaseConverter/Version.hpp

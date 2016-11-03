#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/CppTemplate
printf "#define CPP_TEMPLATE_VERSION \""> ./../../Include/CppTemplate/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/CppTemplate/Version.hpp
printf \""\n" >> ./../../Include/CppTemplate/Version.hpp

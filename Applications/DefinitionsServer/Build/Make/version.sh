#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/DefinitionsServer
printf "#define DEFINITIONS_SERVER_VERSION \""> ./../../Include/DefinitionsServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/DefinitionsServer/Version.hpp
printf \""\n" >> ./../../Include/DefinitionsServer/Version.hpp

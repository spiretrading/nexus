#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/DefinitionsServer
printf "#define DEFINITIONS_SERVER_VERSION \""> ./../../Include/DefinitionsServer/Version.hpp
hg id -n | tr -d "\n" >> ./../../Include/DefinitionsServer/Version.hpp
printf \" >> ./../../Include/DefinitionsServer/Version.hpp

#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/Spire
printf "#define SPIRE_VERSION \""> ./../../Include/Spire/Version.hpp
hg id -n | tr -d "\n" >> ./../../Include/Spire/Version.hpp
printf \" >> ./../../Include/Spire/Version.hpp

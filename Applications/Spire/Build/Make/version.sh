#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/Spire
printf "#define SPIRE_VERSION \""> ./../../Include/Spire/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/Spire/Version.hpp
printf \""\n" >> ./../../Include/Spire/Version.hpp

#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../include/spire
printf "#define SPIRE_VERSION \""> ./../../include/spire/version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../include/spire/version.hpp
printf \""\n" >> ./../../include/spire/version.hpp

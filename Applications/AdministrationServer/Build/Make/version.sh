#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/AdministrationServer
printf "#define ADMINISTRATION_SERVER_VERSION \""> ./../../Include/AdministrationServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/AdministrationServer/Version.hpp
printf \""\n" >> ./../../Include/AdministrationServer/Version.hpp

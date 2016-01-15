#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/AdministrationServer
printf "#define ADMINISTRATION_SERVER_VERSION \""> ./../../Include/AdministrationServer/Version.hpp
hg id -n | tr -d "\n" >> ./../../Include/AdministrationServer/Version.hpp
printf \" >> ./../../Include/AdministrationServer/Version.hpp

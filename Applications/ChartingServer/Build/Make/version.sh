#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/ChartingServer
printf "#define CHARTING_SERVER_VERSION \""> ./../../Include/ChartingServer/Version.hpp
hg id -n | tr -d "\n" >> ./../../Include/ChartingServer/Version.hpp
printf \" >> ./../../Include/ChartingServer/Version.hpp

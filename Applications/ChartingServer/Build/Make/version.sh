#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/ChartingServer
printf "#define CHARTING_SERVER_VERSION \""> ./../../Include/ChartingServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/ChartingServer/Version.hpp
printf \""\n" >> ./../../Include/ChartingServer/Version.hpp

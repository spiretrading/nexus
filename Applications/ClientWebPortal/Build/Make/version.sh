#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/ClientWebPortal
printf "#define CLIENT_WEB_PORTAL_VERSION \""> ./../../Include/ClientWebPortal/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/ClientWebPortal/Version.hpp
printf \""\n" >> ./../../Include/ClientWebPortal/Version.hpp

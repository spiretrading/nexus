#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/WebPortal
printf "#define WEB_PORTAL_VERSION \""> ./../../Include/WebPortal/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/WebPortal/Version.hpp
printf \""\n" >> ./../../Include/WebPortal/Version.hpp

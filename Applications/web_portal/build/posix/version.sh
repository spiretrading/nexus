#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../include/web_portal
printf "#define WEB_PORTAL_VERSION \""> ./../../include/web_portal/version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../include/web_portal/version.hpp
printf \""\n" >> ./../../include/web_portal/version.hpp

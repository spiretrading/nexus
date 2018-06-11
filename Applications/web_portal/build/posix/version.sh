#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../include/client_web_portal
printf "#define CLIENT_WEB_PORTAL_VERSION \""> ./../../include/client_web_portal/version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../include/client_web_portal/version.hpp
printf \""\n" >> ./../../include/client_web_portal/version.hpp

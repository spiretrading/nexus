#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/ComplianceServer
printf "#define COMPLIANCE_SERVER_VERSION \""> ./../../Include/ComplianceServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/ComplianceServer/Version.hpp
printf \""\n" >> ./../../Include/ComplianceServer/Version.hpp

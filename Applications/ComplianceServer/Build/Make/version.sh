#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/ComplianceServer
printf "#define COMPLIANCE_SERVER_VERSION \""> ./../../Include/ComplianceServer/Version.hpp
hg id -n | tr -d "\n" >> ./../../Include/ComplianceServer/Version.hpp
printf \" >> ./../../Include/ComplianceServer/Version.hpp

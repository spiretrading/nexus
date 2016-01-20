#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/RiskServer
printf "#define RISK_SERVER_VERSION \""> ./../../Include/RiskServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/RiskServer/Version.hpp
printf \""\n" >> ./../../Include/RiskServer/Version.hpp

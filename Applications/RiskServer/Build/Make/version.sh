#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/RiskServer
printf "#define RISK_SERVER_VERSION \""> ./../../Include/RiskServer/Version.hpp
hg id -n | tr -d "\n" >> ./../../Include/RiskServer/Version.hpp
printf \" >> ./../../Include/RiskServer/Version.hpp

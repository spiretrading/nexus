#!/bin/bash
DIRECTORY="$(cd -P "$(dirname "${BASH_SOURCE[0]}")" >/dev/null && pwd -P)"
if [[ "${1,,}" == "clean" ]]; then
  rm -rf "Dependencies/library"
  rm -rf "Dependencies/WebApi"
  rm -f "Dependencies/cache_files/nexus_webapi.txt"
  rm -f "Dependencies/cache_files/nexus_webapp.txt"
fi
exec "$DIRECTORY/../../Nexus/build.sh" -D="$DIRECTORY" "$@"

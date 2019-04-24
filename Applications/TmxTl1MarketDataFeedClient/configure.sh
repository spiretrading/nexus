#!/bin/bash
source="${BASH_SOURCE[0]}"
while [ -h "$source" ]; do
  dir="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd)"
  source="$(readlink "$source")"
  [[ $source != /* ]] && source="$dir/$source"
done
directory="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd)"
if [ ! -f "build.bat" ]; then
  echo "\"$directory"/build.sh\" '"$@"' > build.sh
  chmod +x build.sh
fi
cmake $directory "$@"

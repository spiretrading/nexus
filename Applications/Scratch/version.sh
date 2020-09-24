#!/bin/bash
set -o errexit
set -o pipefail
source="${BASH_SOURCE[0]}"
while [ -h "$source" ]; do
  dir="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd -P)"
  source="$(readlink "$source")"
  [[ $source != /* ]] && source="$dir/$source"
done
directory="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd -P)"
if [ ! -f Version.hpp ]; then
  touch Version.hpp
fi
version=$(git --git-dir="$directory/../../.git" rev-list --count --first-parent HEAD)
if ! grep -q $version < Version.hpp; then
  printf "#define SCRATCH_VERSION \""> Version.hpp
  printf $version >> Version.hpp
  printf \" >> Version.hpp
  printf "\n" >> Version.hpp
fi

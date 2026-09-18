#!/bin/bash
set -o errexit
set -o pipefail
config="${1:-Release}"
shopt -s nocasematch
case "$config" in
  release) config="Release" ;;
  debug) config="Debug" ;;
  relwithdebinfo) config="RelWithDebInfo" ;;
  minsizerel) config="MinSizeRel" ;;
  *)
    echo "Error: Invalid configuration \"$config\"."
    exit 1
    ;;
esac
shopt -u nocasematch
directory="$(pwd -P)/../Nexus/Libraries/$config"
for file in aspen.so beam.so nexus.so; do
  if [[ ! -f "$directory/$file" ]]; then
    echo "Error: Source file \"$directory/$file\" not found."
    exit 1
  fi
done
python_directory=$(python3 -m site --user-site 2>/dev/null) || {
  echo "Error: Unable to retrieve Python user-site path."
  exit 1
}
if [[ -z "$python_directory" ]]; then
  echo "Error: Unable to retrieve Python user-site path."
  exit 1
fi
mkdir -p "$python_directory"
for file in aspen.so beam.so nexus.so; do
  cp "$directory/$file" "$python_directory/"
done

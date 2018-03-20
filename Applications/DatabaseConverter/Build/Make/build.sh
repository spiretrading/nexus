#!/bin/bash
set -o errexit
set -o pipefail
directory=$(pwd)
$directory/../../../../Build/Make/local_build.sh $directory $1

#!/bin/bash
# check_server.sh
# 
# Use this script to display the status of WebPortal.
#
# Usage: ./check_server.sh

reset=$(tput sgr0)
red=$(tput setaf 1)
green=$(tput setaf 2)
yellow=$(tput setaf 3)
echo
processes=$(ps -ef | grep -i "WebPortal" | grep -v "grep" | grep -v "bash" | awk '{ print $8 }')
check_run=$(awk -v a="$processes" -v b="WebPortal" 'BEGIN { print index(a, b) }')
if [ "$check_run" = "0" ]; then
  # WebPortal is not running.
  echo "${yellow}WebPortal${reset} is ${red}inactive${reset}."
else
  # WebPortal is running.
  echo "${yellow}WebPortal${reset} is ${green}active${reset}."
fi
echo

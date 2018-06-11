#!/bin/bash
# check_server.sh
# 
# Use this script to display the status of client_web_portal.
#
# Usage: ./check_server.sh

reset=$(tput sgr0)
red=$(tput setaf 1)
green=$(tput setaf 2)
yellow=$(tput setaf 3)
echo
processes=$(ps -ef | grep -i "client_web_portal" | grep -v "grep" | grep -v "bash" | awk '{ print $8 }')
check_run=$(awk -v a="$processes" -v b="client_web_portal" 'BEGIN { print index(a, b) }')
if [ "$check_run" = "0" ]; then
  # client_web_portal is not running.
  echo "${yellow}client_web_portal${reset} is ${red}inactive${reset}."
else
  # client_web_portal is running.
  echo "${yellow}client_web_portal${reset} is ${green}active${reset}."
fi
echo

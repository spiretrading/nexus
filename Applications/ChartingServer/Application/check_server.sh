#!/bin/bash
# check_server.sh
# 
# Use this script to display the status of ChartingServer.
#
# Usage: ./check_server.sh

reset=$(tput sgr0)
red=$(tput setaf 1)
green=$(tput setaf 2)
yellow=$(tput setaf 3)
echo
processes=$(ps -ef | grep -i "ChartingServer" | grep -v "grep" | grep -v "bash" | awk '{ print $8 }')
check_run=$(awk -v a="$processes" -v b="ChartingServer" 'BEGIN { print index(a, b) }')
if [ "$check_run" = "0" ]; then
  # ChartingServer is not running.
  echo "${yellow}ChartingServer${reset} is ${red}inactive${reset}."
else
  # ChartingServer is running.
  echo "${yellow}ChartingServer${reset} is ${green}active${reset}."
fi
echo

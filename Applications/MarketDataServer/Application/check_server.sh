#!/bin/bash
# check_server.sh
# 
# Use this script to display the status of MarketDataServer.
#
# Usage: ./check_server.sh

reset=$(tput sgr0)
red=$(tput setaf 1)
green=$(tput setaf 2)
yellow=$(tput setaf 3)
echo
processes=$(ps -ef | grep -i "MarketDataServer" | grep -v "grep" | grep -v "bash" | awk '{ print $8 }')
check_run=$(awk -v a="$processes" -v b="MarketDataServer" 'BEGIN { print index(a, b) }')
if [ "$check_run" = "0" ]; then
  # MarketDataServer is not running.
  echo "${yellow}MarketDataServer${reset} is ${red}inactive${reset}."
else
  # MarketDataServer is running.
  echo "${yellow}MarketDataServer${reset} is ${green}active${reset}."
fi
echo

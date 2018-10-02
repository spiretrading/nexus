#!/bin/bash
# check_server.sh
# 
# Use this script to display the status of ReplayMarketDataFeedClient.
#
# Usage: ./check_server.sh

reset=$(tput sgr0)
red=$(tput setaf 1)
green=$(tput setaf 2)
yellow=$(tput setaf 3)
echo
processes=$(ps -ef | grep -i "ReplayMarketDataFeedClient" | grep -v "grep" | grep -v "bash" | awk '{ print $8 }')
check_run=$(awk -v a="$processes" -v b="ReplayMarketDataFeedClient" 'BEGIN { print index(a, b) }')
if [ "$check_run" = "0" ]; then
  # ReplayMarketDataFeedClient is not running.
  echo "${yellow}ReplayMarketDataFeedClient${reset} is ${red}inactive${reset}."
else
  # ReplayMarketDataFeedClient is running.
  echo "${yellow}ReplayMarketDataFeedClient${reset} is ${green}active${reset}."
fi
echo

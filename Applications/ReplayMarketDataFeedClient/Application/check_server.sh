#!/bin/bash
# check_server.sh
# 
# Use this script to display the status of MySqlDumpMarketDataFeedClient.
#
# Usage: ./check_server.sh

reset=$(tput sgr0)
red=$(tput setaf 1)
green=$(tput setaf 2)
yellow=$(tput setaf 3)
echo
processes=$(ps -ef | grep -i "MySqlDumpMarketDataFeedClient" | grep -v "grep" | grep -v "bash" | awk '{ print $8 }')
check_run=$(awk -v a="$processes" -v b="MySqlDumpMarketDataFeedClient" 'BEGIN { print index(a, b) }')
if [ "$check_run" = "0" ]; then
  # MySqlDumpMarketDataFeedClient is not running.
  echo "${yellow}MySqlDumpMarketDataFeedClient${reset} is ${red}inactive${reset}."
else
  # MySqlDumpMarketDataFeedClient is running.
  echo "${yellow}MySqlDumpMarketDataFeedClient${reset} is ${green}active${reset}."
fi
echo

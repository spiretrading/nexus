#!/bin/bash
# start_server.sh
# 
# Use this script to start the MySqlDumpMarketDataFeedClient.
#
# Usage: ./start_server.sh    Starts the MySqlDumpMarketDataFeedClient.

reset=$(tput sgr0)
red=$(tput setaf 1)
green=$(tput setaf 2)
yellow=$(tput setaf 3)
echo
directory=$(ls -l)
check_exist=$(awk -v a="$directory" -v b="MySqlDumpMarketDataFeedClient" 'BEGIN { print index(a, b) }')
if [ "$check_exist" = "0" ]; then
  # MySqlDumpMarketDataFeedClient is not present.
  echo "${red}[ERROR]${reset} Could not start ${yellow}MySqlDumpMarketDataFeedClient${reset}."
  echo "        ${yellow}MySqlDumpMarketDataFeedClient${reset} could not be found."
else
  # MySqlDumpMarketDataFeedClient is present.
  leftover=$(ls srv_*.log 2>/dev/null)
  if [ -n "leftover" ]; then
    if [ ! -d "logs" ]; then
      mkdir logs
    fi
    for var in $leftover; do
      mv $var logs
    done
  fi
  processes=$(ps -ef | grep -i "MySqlDumpMarketDataFeedClient" | grep -v "grep" | grep -v "bash" | awk '{ print $8 }')
  check_run=$(awk -v a="$processes" -v b="MySqlDumpMarketDataFeedClient" 'BEGIN { print index(a, b) }')
  if [ "$check_run" = "0" ]; then
    date_time=$(date '+%Y%m%d_%H_%M_%S')
    new_log_name="srv_$date_time.log"
    ./MySqlDumpMarketDataFeedClient &> $new_log_name &
    echo "${yellow}MySqlDumpMarketDataFeedClient${reset} has been started."
  else
    # MySqlDumpMarketDataFeedClient is already running.
    echo "${red}[ERROR]${reset} Could not start ${yellow}MySqlDumpMarketDataFeedClient${reset}."
    echo "        ${yellow}MySqlDumpMarketDataFeedClient${reset} is already running."
  fi
fi
echo

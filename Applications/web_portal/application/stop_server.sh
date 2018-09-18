#!/bin/bash
# stop_server.sh
# 
# Use this script to stop a web_portal.
#
# Usage: ./stop_server.sh    Stops the web_portal.

reset=$(tput sgr0)
red=$(tput setaf 1)
green=$(tput setaf 2)
yellow=$(tput setaf 3)
echo
# web_portal is present.
processes=$(ps -ef | grep -i "web_portal" | grep -v "grep" | grep -v "bash" | awk '{ print $8 }')
check_run=$(awk -v a="$processes" -v b="web_portal" 'BEGIN { print index(a, b) }')
if [ "$check_run" = "0" ]; then
  # web_portal is not running.
  echo "${red}[ERROR]${reset} Could not terminate ${yellow}web_portal${reset}."
  echo "        ${yellow}web_portal${reset} is not active."
else
  # web_portal is already running.
  pid=$(ps -ef | grep -i "web_portal" | grep -v "grep" | grep -v "bash" | awk '{ print $2 }')
  kill -s INT $pid
  echo "${yellow}web_portal${reset} terminated."
  srv_logs=$(ls srv_*.log 2>/dev/null)
  if [ -n "srv_logs" ]; then
    if [ ! -d "logs" ]; then
      mkdir logs
    fi
    for var in $srv_logs; do
      mv $var logs
    done
  else
    echo "${red}[ERROR]${reset} No log file could be found."
  fi
fi
echo

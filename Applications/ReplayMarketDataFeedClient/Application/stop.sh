#!/bin/bash
APPLICATION="ReplayMarketDataFeedClient"
PID_FILE="pid.lock"

if [[ -f "$PID_FILE" ]]; then
  pid=$(<"$PID_FILE")
  if ./check.sh > /dev/null; then
    kill -SIGINT "$pid" 2> /dev/null
    timeout 300s wait "$pid" 2> /dev/null
    if [[ $? -eq 124 ]]; then
      kill -SIGKILL "$pid" > /dev/null
      log_file=$(ls -t srv_*.log 2>/dev/null | head -n 1)
      if [[ -n "$log_file" ]]; then
       echo "Forcefully terminated $APPLICATION." >> "$log_file"
      fi
    fi
  fi
  rm -f "$PID_FILE"
fi

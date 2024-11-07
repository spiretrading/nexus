#!/bin/bash
APPLICATION="WebPortal"
PID_FILE="pid.lock"

wait_for_termination() {
  local pid=$1
  local timeout=300
  local interval=0.1
  local max_interval=10
  local elapsed=0
  while(( $(echo "$elapsed < $timeout" | bc -l) )); do
    if [[ ! -e /proc/$pid ]]; then
      return 0
    fi
    sleep $interval
    elapsed=$(echo "$elapsed + $interval" | bc)
    interval=$(echo "$interval * 2" | bc)
    if(( $(echo "$interval > $max_interval" | bc -l) )); then
      interval=$max_interval
    fi
  done
  return 1
}

if [[ -f "$PID_FILE" ]]; then
  pid=$(<"$PID_FILE")
  if ./check.sh > /dev/null; then
    kill -SIGINT "$pid" 2> /dev/null
    if ! wait_for_termination "$pid"; then
      kill -SIGKILL "$pid" > /dev/null
      log_file=$(ls -t srv_*.log 2>/dev/null | head -n 1)
      if [[ -n "$log_file" ]]; then
       echo "Forcefully terminated $APPLICATION." >> "$log_file"
      fi
    fi
  fi
  rm -f "$PID_FILE"
fi

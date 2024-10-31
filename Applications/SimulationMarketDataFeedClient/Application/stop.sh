#!/bin/bash
APPLICATION="SimulationMarketDataFeedClient"
CHECK_SCRIPT="./check.sh"
PID_FILE="pid.lock"
log_file=$(ls -t srv_*.log 2>/dev/null | head -n 1)

is_application_running() {
  $CHECK_SCRIPT > /dev/null
  return $?
}

stop_application() {
  local pid=$1
  local elapsed_time=0
  local interval=2
  while (( elapsed_time < 300 )); do
    kill -SIGINT "$pid"
    sleep "$interval"
    if ! ps -p "$pid" > /dev/null 2>&1; then
      return 0
    fi
    interval=10
    elapsed_time=$(( elapsed_time + interval ))
  done
  kill -SIGKILL "$pid"
  if [[ -n "$log_file" ]]; then
    echo "Forcefully terminated $APPLICATION." >> "$log_file"
  fi
}

if [[ -f "$PID_FILE" ]]; then
  if is_application_running; then
    pid=$(<"$PID_FILE")
    stop_application "$pid"
  fi
  rm -f "$PID_FILE"
fi

#!/bin/bash
APPLICATION="MarketDataServer"
PID_FILE="pid.lock"
LOG_DIR="./logs"

is_process_running() {
  local pid=$1
  ps -p "$pid" > /dev/null 2>&1
  return $?
}

mkdir -p "$LOG_DIR"
date_time=$(date '+%Y%m%d_%H_%M_%S')
log_name="srv_$date_time.log"
for existing_log in srv_*.log; do
  if [[ -f "$existing_log" ]]; then
    mv "$existing_log" "$LOG_DIR"
  fi
done
if [[ -f "$PID_FILE" ]]; then
  existing_pid=$(<"$PID_FILE")
  if is_process_running "$existing_pid"; then
    echo "$APPLICATION is already running with PID $existing_pid."
    exit 0
  else
    echo "Found stale PID file. Removing and starting a new instance."
    rm -f "$PID_FILE"
  fi
fi
./$APPLICATION > "$log_name" 2>&1 &
new_pid=$!
echo "$new_pid" > "$PID_FILE"
